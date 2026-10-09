#!/usr/bin/env python3
"""Generate small native DAIMOS Makefiles from KCC's authoritative link sets.

The native MAKE has fixed-size arenas.  One file per phase avoids enlarging
those arenas just to parse a monolithic KCC dependency graph.  The generator
does not compile anything or silently substitute bootstrap DXRs for outputs.
"""

import pathlib
import re
import sys

if len(sys.argv) != 3:
    raise SystemExit("usage: gen-kcc-native-make-v1.py KCC-REPO OUT-DIR")

repo = pathlib.Path(sys.argv[1])
out = pathlib.Path(sys.argv[2])
out.mkdir(parents=True, exist_ok=True)
text = (repo / "Makefile").read_text().replace("\\\n", " ")
variables = dict(re.findall(r"(?m)^([A-Z][A-Z0-9_]*)\s*=\s*(.*)$", text))

def expand(s):
    for _ in range(12):
        # GNU/BSD make suffix replacement in the authoritative object lists:
        # $(MODULES:.c=.dobj).  Resolve before ordinary $(NAME) expansion.
        s = re.sub(
            r"\$\(([A-Z][A-Z0-9_]*):([^=():]+)=([^():]+)\)",
            lambda m: " ".join(
                word[:-len(m.group(2))] + m.group(3)
                if word.endswith(m.group(2)) else word
                for word in variables.get(m.group(1), "").split()), s)
        replaced = re.sub(r"\$\(([A-Z][A-Z0-9_]*)\)",
                          lambda m: variables.get(m.group(1), ""), s)
        if replaced == s:
            return s
        s = replaced
    raise ValueError("recursive variables: " + s)

sources = {}
profiles = {}
for obj, source in re.findall(
    r"(?m)^\$\(NATIVE_BUILD_DIR\)/([a-z0-9-]+)-v1\.s:\s+([a-z0-9]+)\.c", text
):
    sources[obj] = source
for obj, source, profile in re.findall(
    r"(?m)^\$\(NATIVE_BUILD_DIR\)/([a-z0-9-]+)-v1\.s:\s+([a-z0-9]+)\.c[^\n]*\n[^\n]*-DKCC_PHASE_([A-Z]+)=1", text
):
    profiles[obj] = profile

common_flags = ("-Pgnu99 -x=pdp6 -m=gas "
                "-DHOST_DAIMOS=1 -DHOST_UNIX=0 "
                "-I/OPTION/SOURCE/KCC/SELF/INCLUDE "
                "-I/OPTION/SOURCE/KCC/ABI")
build = "B"
kcc = "/OPTION/BASE/EXEC/KCC"
das = "/OPTION/BASE/EXEC/DAS"
darc = "/OPTION/BASE/EXEC/DARC"
dlink = "/OPTION/BASE/EXEC/DLINK"
install = "/SYSTEM/EXEC/INSTALL"
rm = "/SYSTEM/EXEC/RM"
runtime = ["CRT0.DOBJ", "BOOT.DOBJ", "SYS.DOBJ", "HELP.DOBJ"]

def add_rule(lines, target, deps, commands):
    # Make's line buffer is 256 characters; split long prerequisites by
    # spelling each prerequisite on a separate (coalesced) target rule.
    if deps:
        for dep in deps:
            line = f"{target}: {dep}"
            if len(line) > 255:
                raise ValueError(line)
            lines.append(line)
    else:
        lines.append(target + ":")
    for command in commands:
        if len(command) > 252:
            raise ValueError("recipe too long: " + command)
        lines.append("> !" + command)

def object_rule(name, profile):
    lower = name.lower()
    if lower.endswith("-v1.dobj"):
        lower = lower[:-8]
    if lower in ("daimos-chain", "daimos-path", "daimos-driver"):
        source = "RUNTIME/" + lower.upper() + ".C"
    else:
        source = sources.get(lower, lower.split("-")[0]).upper() + ".C"
    target = f"{build}/{name}"
    asm = target[:-5] + ".S"
    # Host-proven per-object overrides are in the KCC Makefile.  Objects
    # carrying -CPP/-PARSE/-GEN/-OPT/-CORE are profile-specific variants.
    opt_profile = profile
    for part, value in (("-CPP", "CPP"), ("-PARSE", "PARSE"),
                        ("-GEN", "GEN"), ("-OPT", "OPT"),
                        ("-CORE", "CORE")):
        if part in name:
            opt_profile = value
            break
    opt_profile = profiles.get(lower, opt_profile)
    opts = common_flags + (" -DKCC_PHASE_" + opt_profile + "=1" if lower in profiles else "")
    return target, source, asm, opts

phases = (("KCPP", "NATIVE_KCPP_OBJS"),
          ("KPARSE", "NATIVE_KPARSE_OBJS"),
          ("KGEN", "NATIVE_KGEN_OBJS"),
          ("KOPT", "NATIVE_KOPT_OBJS"))

for phase, var in phases:
    objs = []
    for raw in expand(variables[var]).split():
        name = pathlib.PurePosixPath(raw).name.upper()
        if not name.endswith(".DOBJ"):
            raise ValueError((phase, raw))
        objs.append(name)
    lines = ["# PHASE LINK WITH INCREMENTAL OBJECT SUBBUILDS.", ".PHONY: ALL CLEAN", "ALL:"]
    groups = [objs[i:i + 4] for i in range(0, len(objs), 4)]
    archives = []
    for i, group in enumerate(groups):
        sub = ["# SMALL PHASE OBJECT SUBGRAPH.", ".PHONY: ALL CLEAN", "ALL: " + build + "/" + phase + str(i + 1) + ".DARC"]
        for name in dict.fromkeys(group):
            target, source, asm, flags = object_rule(name, phase.replace("K", "", 1))
            add_rule(sub, asm, [source], [f"{kcc} -S -O {asm} {flags} {source}"])
            add_rule(sub, target, [asm], [f"{das} -F -C -O {target} {asm}"])
        target = f"{build}/{phase}{i + 1}.DARC"
        archives.append(target)
        add_rule(sub, target, [f"{build}/{n}" for n in group],
                 [f"{darc} -O {target} " + " ".join(f"{build}/{n}" for n in group)])
        sub.append("CLEAN:")
        for name in dict.fromkeys(group):
            target, _, asm, _ = object_rule(name, phase.replace("K", "", 1))
            sub.extend([f"> -!{rm} {asm}", f"> -!{rm} {target}"])
        sub.append(f"> -!{rm} B/{phase}{i + 1}.DARC")
        filename = "M" + phase[1:] + str(i + 1)
        (out / filename).write_text("\n".join(sub) + "\n")
        lines.append(f"> !/SYSTEM/EXEC/MAKE -F {filename} ALL")
    lines.append("> !/SYSTEM/EXEC/MAKE -F L" + phase[1:] + " ALL")
    lines.append("CLEAN:")
    lines.append("> !/SYSTEM/EXEC/MAKE -F L" + phase[1:] + " CLEAN")
    for i in range(len(groups)):
        lines.append(f"> !/SYSTEM/EXEC/MAKE -F M{phase[1:]}{i + 1} CLEAN")
    link = ["# PHASE LINK GRAPH.", ".PHONY: ALL", "ALL: " + build + "/" + phase + ".DXR"]
    target = f"{build}/{phase}.DXR"
    dep = archives + ["BOOT/" + r for r in runtime] + ["BOOT/LIBC.DARC"]
    args = " ".join("BOOT/" + r for r in runtime)
    args += " " + " ".join(archives) + " BOOT/LIBC.DARC"
    add_rule(link, target, dep, [f"{dlink} --DAIMOS-UUO-RELAX -B 020 -O {target} {args}"])
    link.extend(["CLEAN:", f"> -!{rm} {target}"])
    (out / ("L" + phase[1:])).write_text("\n".join(link) + "\n")
    (out / ("MK" + phase[1:])).write_text("\n".join(lines) + "\n")

# The KCC command driver is rebuilt from source too.  Its runtime bootstrap
# and ABI library are seeded separately by the boot-image construction.
drv = ["# NATIVE KCC DRIVER OBJECT AND DXR.", ".PHONY: ALL", "ALL: B/KCC.DXR"]
drv_source = "RUNTIME/DAIMOS-DRIVER.C"
drv_asm = "B/DAIMOS-DRIVER.S"
drv_obj = "B/DAIMOS-DRIVER.DOBJ"
add_rule(drv, drv_asm, [drv_source],
         [f"{kcc} -S -O {drv_asm} {common_flags} {drv_source}"])
add_rule(drv, drv_obj, [drv_asm],
         [f"{das} -F -C -O {drv_obj} {drv_asm}"])
add_rule(drv, "B/KCC.DXR", [drv_obj, "BOOT/CRT0.DOBJ", "BOOT/BOOT.DOBJ",
                            "BOOT/SYS.DOBJ", "BOOT/HELP.DOBJ", "BOOT/LIBC.DARC"],
         [f"{dlink} --DAIMOS-UUO-RELAX -B 020 -O B/KCC.DXR "
          "BOOT/CRT0.DOBJ BOOT/BOOT.DOBJ BOOT/SYS.DOBJ BOOT/HELP.DOBJ "
          "B/DAIMOS-DRIVER.DOBJ BOOT/LIBC.DARC"])
drv.extend(["CLEAN:", f"> -!{rm} B/KCC.DXR",
            f"> -!{rm} {drv_obj}", f"> -!{rm} {drv_asm}"])
(out / "MKDRV").write_text("\n".join(drv) + "\n")

top = ["# NATIVE KCC REBUILD: EACH PHASE IS A SEPARATE SMALL MAKE GRAPH.",
       ".PHONY: ALL INSTALL HELP CLEAN", "ALL:",
       f"> !{install} -D -M 0777 {build}"]
for phase, _ in phases:
    top.append(f"> !/SYSTEM/EXEC/MAKE -F MK{phase[1:]} ALL")
top.append("> !/SYSTEM/EXEC/MAKE -F MKDRV ALL")
top += ["HELP:", "> !/SYSTEM/EXEC/ECHO MAKE ALL BUILDS KCPP KPARSE KGEN KOPT",
        "INSTALL: ALL",
        f"> !{install} -D -M 0755 /OPTION/BASE/LIBEXEC/KCC"]
for phase, _ in phases:
    top.append(f"> !{install} -M 0555 {build}/{phase}.DXR /OPTION/BASE/LIBEXEC/KCC/{phase}")
top.append(f"> !{install} -M 0555 B/KCC.DXR /OPTION/BASE/EXEC/KCC")
top.append("CLEAN:")
for phase, _ in phases:
    top.append(f"> -!{rm} B/{phase}.DXR")
    count = len(expand(variables[dict(phases)[phase]]).split())
    for i in range((count + 3) // 4):
        top.append(f"> !/SYSTEM/EXEC/MAKE -F M{phase[1:]}{i + 1} CLEAN")
top.append("> !/SYSTEM/EXEC/MAKE -F MKDRV CLEAN")
(out / "MAKEFILE").write_text("\n".join(top) + "\n")

# Convert the authoritative KCC object closure into ONE normal dependency
# graph.  Split files above remain implementation intermediates only; they
# are NOT staged and the native build does not recurse through subprocesses.
single = ["# GENERATED FROM KCC/MAKEFILE NATIVE OBJECT SETS.",
          "KCC = /OPTION/BASE/EXEC/KCC",
          "DAS = /OPTION/BASE/EXEC/DAS",
          "DARC = /OPTION/BASE/EXEC/DARC",
          "DLINK = /OPTION/BASE/EXEC/DLINK",
          "INSTALL = /SYSTEM/EXEC/INSTALL",
          "RM = /SYSTEM/EXEC/RM",
          "KFLAGS = " + common_flags,
          ".PHONY: ALL INSTALL CLEAN HELP",
          "ALL: B B/KCPP.DXR B/KPARSE.DXR B/KGEN.DXR B/KOPT.DXR B/KCC.DXR"]
add_rule(single, "B", [], [f"{install} -D -M 0777 B"])
seen_rules = set()
all_generated = set()
for phase, var in phases:
    for name in dict.fromkeys(pathlib.PurePosixPath(raw).name.upper()
                              for raw in expand(variables[var]).split()):
        if name in seen_rules:
            continue
        seen_rules.add(name)
        target, source, asm, flags = object_rule(name, phase.replace("K", "", 1))
        add_rule(single, asm, [source], [f"{kcc} -S -O {asm} {flags} {source}"])
        add_rule(single, target, [asm], [f"{das} -F -C -O {target} {asm}"])
        all_generated.update((asm, target))
    objs = [pathlib.PurePosixPath(raw).name.upper()
            for raw in expand(variables[var]).split()]
    archives = []
    for i in range(0, len(objs), 4):
        group = objs[i:i + 4]
        archive = f"B/{phase}{i // 4 + 1}.DARC"
        archives.append(archive)
        add_rule(single, archive, ["B/" + name for name in group],
                 [f"{darc} -O {archive} " + " ".join("B/" + n for n in group)])
        all_generated.add(archive)
    target = f"B/{phase}.DXR"
    dep = archives + ["BOOT/" + r for r in runtime] + ["BOOT/LIBC.DARC"]
    args = " ".join("BOOT/" + r for r in runtime)
    args += " " + " ".join(archives) + " BOOT/LIBC.DARC"
    add_rule(single, target, dep, [f"{dlink} --DAIMOS-UUO-RELAX -B 020 -O {target} {args}"])
    all_generated.add(target)

add_rule(single, drv_asm, [drv_source],
         [f"{kcc} -S -O {drv_asm} {common_flags} {drv_source}"])
add_rule(single, drv_obj, [drv_asm], [f"{das} -F -C -O {drv_obj} {drv_asm}"])
add_rule(single, "B/KCC.DXR", [drv_obj] + ["BOOT/" + r for r in runtime]
         + ["BOOT/LIBC.DARC"],
         [f"{dlink} --DAIMOS-UUO-RELAX -B 020 -O B/KCC.DXR "
          "BOOT/CRT0.DOBJ BOOT/BOOT.DOBJ BOOT/SYS.DOBJ BOOT/HELP.DOBJ "
          "B/DAIMOS-DRIVER.DOBJ BOOT/LIBC.DARC"])
all_generated.update((drv_asm, drv_obj, "B/KCC.DXR"))
single.extend(["INSTALL: ALL",
               f"> !{install} -D -M 0755 /OPTION/BASE/LIBEXEC/KCC"])
for phase, _ in phases:
    single.append(f"> !{install} -M 0555 B/{phase}.DXR /OPTION/BASE/LIBEXEC/KCC/{phase}")
single.append(f"> !{install} -M 0555 B/KCC.DXR /OPTION/BASE/EXEC/KCC")
single.append("CLEAN:")
# RM accepts multiple paths. Batch to avoid launching one PDP-6 process per
# file; seven entries keep argv, recipe length and RUN V2 within bounds.
clean_files = sorted(all_generated)
for i in range(0, len(clean_files), 7):
    single.append("> -!" + rm + " " + " ".join(clean_files[i:i + 7]))
single.extend(["HELP:", "> !/SYSTEM/EXEC/ECHO MAKE ALL INSTALL CLEAN"])
for i, line in enumerate(single):
    if line.startswith("> "):
        for original, variable in ((kcc, "KCC"), (das, "DAS"),
                                   (darc, "DARC"), (dlink, "DLINK"),
                                   (install, "INSTALL"), (rm, "RM")):
            line = line.replace(original, "$(" + variable + ")")
        line = line.replace(common_flags, "$(KFLAGS)")
        single[i] = line
# Retain the large graph for host-side diagnostics only.  Native MAKE has
# bounded tables and must consume the small phase-specific graphs above.
(out / "MONOLITH").write_text("\n".join(single) + "\n")
print("generated KCC native Makefiles:", ", ".join(p.name for p in out.iterdir()))
