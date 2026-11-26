"""Run from any directory with Python 3 and GCC on PATH. No STM32 board needed."""
import os
from pathlib import Path
import subprocess
import tempfile

tests = Path(__file__).resolve().parent
core = tests.parent / "Core"
with tempfile.TemporaryDirectory() as temp:
    for lesson in (1, 2, 3):
        exe = Path(temp) / ("test.exe" if os.name == "nt" else "test")
        subprocess.run(["gcc", "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
                        f"-DPIF_LESSON={lesson}", f"-I{core/'Inc'}", "-I"+str(tests/"mock"),
                        *[str(core/"Src"/(name+".c")) for name in ("motor","sensor","filter","pid","controller")], str(tests/"test.c"),
                        "-lm", "-o", str(exe)], check=True)
        subprocess.run([str(exe)], check=True)
        if lesson == 3:
            subprocess.run([str(exe), "inverted"], check=True)
            for ahead, track, lag in [(45, 105, 0.055), (35, 115, 0.07), (55, 95, 0.04)]:
                print(f"Sensor ahead {ahead} mm, axle width {track} mm, motor lag {lag} s", flush=True)
                for path in ("waves", "noise", "tight"):
                    subprocess.run([str(exe), path, str(ahead), str(track), str(lag)], check=True)

    for ki, kd in ((0, 0), (2, 0.12)):
        subprocess.run(["gcc", "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
                        f"-DKI={ki}", f"-DKD={kd}", f"-I{core/'Inc'}",
                        str(core/"Src"/"pid.c"), str(tests/"pid_test.c"), "-lm",
                        "-o", str(exe)], check=True)
        subprocess.run([str(exe)], check=True)
        print(f"PASS pid test: KI={ki}, KD={kd}")
