#!/usr/bin/env python3
"""Socket-backed functional test for the resident PDP-6 DCS driver."""

import argparse
import os
import socket
import subprocess
import tempfile
import time

EXIT_TIMEOUT = 12.0


def free_port():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(("127.0.0.1", 0))
    port = sock.getsockname()[1]
    sock.close()
    return port


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--simh", required=True)
    parser.add_argument("--ini", required=True)
    parser.add_argument("--log", required=True)
    args = parser.parse_args()

    port = free_port()
    text = open(args.ini, "r", encoding="ascii").read()
    if "set dcs disabled" not in text:
        raise SystemExit("DCS test: base INI does not disable DCS")
    text = text.replace("set dcs disabled", "set dcs enabled\nset dcs lines=8")
    text = text.replace("go 020", "attach -U dcs 127.0.0.1:%d\ngo 020" % port)

    fd, ini_path = tempfile.mkstemp(prefix="daimos-dcs-", suffix=".ini")
    os.close(fd)
    with open(ini_path, "w", encoding="ascii") as fp:
        fp.write(text)

    proc = subprocess.Popen(
        [args.simh, ini_path], stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )
    dummy = None
    client = None
    dummy_received = b""
    received = b""
    try:
        deadline = time.time() + 12.0
        while time.time() < deadline:
            try:
                dummy = socket.create_connection(("127.0.0.1", port), timeout=0.25)
                break
            except OSError:
                if proc.poll() is not None:
                    break
                time.sleep(0.05)
        if dummy is None:
            raise RuntimeError("DCS test: could not connect line 0")
        dummy.settimeout(0.1)
        time.sleep(0.15)
        client = socket.create_connection(("127.0.0.1", port), timeout=1.0)
        client.settimeout(0.25)
        time.sleep(0.15)
        client.sendall(b"R")

        while time.time() < deadline and b"DAIMOS DCS1 OK" not in received:
            try:
                chunk = client.recv(4096)
                if chunk:
                    received += chunk
                elif proc.poll() is not None:
                    break
            except socket.timeout:
                if proc.poll() is not None:
                    break

        try:
            output, _ = proc.communicate(timeout=EXIT_TIMEOUT)
        except subprocess.TimeoutExpired:
            proc.kill()
            output, _ = proc.communicate()
            with open(args.log, "wb") as fp:
                fp.write(output or b"")
                fp.write(b"\n--- TIMEOUT DCS LINE 0 ---\n")
                fp.write(dummy_received)
                fp.write(b"\n--- TIMEOUT DCS LINE 1 ---\n")
                fp.write(received)
            raise RuntimeError("DCS test: simulator did not finish")

        try:
            while True:
                chunk = dummy.recv(4096)
                if not chunk:
                    break
                dummy_received += chunk
        except socket.timeout:
            pass

        with open(args.log, "wb") as fp:
            fp.write(output or b"")
            fp.write(b"\n--- DCS LINE 0 ---\n")
            fp.write(dummy_received)
            fp.write(b"\n--- DCS LINE 1 ---\n")
            fp.write(received)

        if b"DCS                                   OK" not in (output or b""):
            raise RuntimeError("DCS test: successful MINIT diagnostic missing")
        if b"HALT instruction" not in (output or b""):
            raise RuntimeError("DCS test: kernel did not reach HALT")
        if b"DAIMOS DCS1 OK" in dummy_received:
            raise RuntimeError("DCS test: line-1 output was misrouted to line 0")
        if b"DAIMOS DCS1 OK" not in received:
            raise RuntimeError("DCS test: line-1 socket output missing")
        print("PDP-6 DCS socket test PASS")
        return 0
    finally:
        if client is not None:
            client.close()
        if dummy is not None:
            dummy.close()
        if proc.poll() is None:
            proc.kill()
            proc.wait()
        try:
            os.unlink(ini_path)
        except OSError:
            pass


if __name__ == "__main__":
    raise SystemExit(main())
