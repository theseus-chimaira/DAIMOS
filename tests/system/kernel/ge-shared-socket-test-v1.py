#!/usr/bin/env python3
"""Socket test for shared DCS + GE/GTY PI4 dispatch."""

import argparse
import os
import socket
import subprocess
import tempfile
import time


def free_port():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(("127.0.0.1", 0))
    port = sock.getsockname()[1]
    sock.close()
    return port


def connect(port, deadline, proc, what):
    while time.time() < deadline:
        try:
            sock = socket.create_connection(("127.0.0.1", port), timeout=0.25)
            sock.settimeout(0.1)
            return sock
        except OSError:
            if proc.poll() is not None:
                break
            time.sleep(0.05)
    raise RuntimeError("GE shared test: could not connect " + what)


def drain(sock):
    data = b""
    try:
        while True:
            chunk = sock.recv(4096)
            if not chunk:
                break
            data += chunk
    except socket.timeout:
        pass
    return data


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--simh", required=True)
    parser.add_argument("--ini", required=True)
    parser.add_argument("--log", required=True)
    args = parser.parse_args()

    dcs_port = free_port()
    ge_port = free_port()
    text = open(args.ini, "r", encoding="ascii").read()
    if "set dcs disabled" not in text or "set ge disabled" not in text:
        raise SystemExit("GE shared test: base INI must disable DCS and GE")
    text = text.replace("set dcs disabled", "set dcs enabled\nset dcs lines=8")
    text = text.replace("set ge disabled", "set ge enabled")
    text = text.replace(
        "go 020",
        "attach -U dcs 127.0.0.1:%d\nattach -U ge 127.0.0.1:%d\ngo 020"
        % (dcs_port, ge_port),
    )

    fd, ini_path = tempfile.mkstemp(prefix="daimos-ge-shared-", suffix=".ini")
    os.close(fd)
    with open(ini_path, "w", encoding="ascii") as fp:
        fp.write(text)

    proc = subprocess.Popen(
        [args.simh, ini_path], stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )
    sockets = []
    output = b""
    try:
        deadline = time.time() + 15.0
        dcs0 = connect(dcs_port, deadline, proc, "DCS line 0")
        sockets.append(dcs0)
        time.sleep(0.10)
        dcs1 = connect(dcs_port, deadline, proc, "DCS line 1")
        sockets.append(dcs1)
        ge0 = connect(ge_port, deadline, proc, "GE line 0")
        sockets.append(ge0)
        time.sleep(0.10)
        ge1 = connect(ge_port, deadline, proc, "GE line 1")
        sockets.append(ge1)
        time.sleep(0.15)

        dcs1.sendall(b"R")
        ge1.sendall(b"G")

        dcs_out = b""
        ge_out = b""
        while time.time() < deadline:
            for sock, which in ((dcs1, "dcs"), (ge1, "ge")):
                try:
                    chunk = sock.recv(4096)
                    if chunk:
                        if which == "dcs":
                            dcs_out += chunk
                        else:
                            ge_out += chunk
                except socket.timeout:
                    pass
            if b"DAIMOS DCS1 OK" in dcs_out and b"DAIMOS GE1 OK" in ge_out:
                break
            if proc.poll() is not None:
                break

        try:
            output, _ = proc.communicate(timeout=max(0.5, deadline - time.time()))
        except subprocess.TimeoutExpired:
            proc.kill()
            output, _ = proc.communicate()
            with open(args.log, "wb") as fp:
                fp.write(output or b"")
                fp.write(b"\n--- TIMEOUT DCS1 ---\n" + dcs_out)
                fp.write(b"\n--- TIMEOUT GE1 ---\n" + ge_out)
            raise RuntimeError("GE shared test: simulator did not finish")

        dcs0_out = drain(dcs0)
        ge0_out = drain(ge0)
        with open(args.log, "wb") as fp:
            fp.write(output or b"")
            fp.write(b"\n--- DCS0 ---\n" + dcs0_out)
            fp.write(b"\n--- DCS1 ---\n" + dcs_out)
            fp.write(b"\n--- GE0 ---\n" + ge0_out)
            fp.write(b"\n--- GE1 ---\n" + ge_out)

        if b"DCS                                   OK" not in (output or b""):
            raise RuntimeError("GE shared test: DCS MINIT diagnostic missing")
        if b"GE                                    OK" not in (output or b""):
            raise RuntimeError("GE shared test: GE MINIT diagnostic missing")
        if b"HALT instruction" not in (output or b""):
            raise RuntimeError("GE shared test: kernel did not reach HALT")
        if b"DAIMOS DCS1 OK" not in dcs_out:
            raise RuntimeError("GE shared test: DCS line-1 output missing")
        if b"DAIMOS GE1 OK" not in ge_out:
            raise RuntimeError("GE shared test: GE line-1 output missing")
        if b"DAIMOS DCS1 OK" in dcs0_out or b"DAIMOS GE1 OK" in ge0_out:
            raise RuntimeError("GE shared test: terminal output misrouted")
        print("PDP-6 shared DCS/GE PI4 socket test PASS")
        return 0
    finally:
        for sock in sockets:
            sock.close()
        if proc.poll() is None:
            proc.kill()
            proc.wait()
        try:
            os.unlink(ini_path)
        except OSError:
            pass


if __name__ == "__main__":
    raise SystemExit(main())
