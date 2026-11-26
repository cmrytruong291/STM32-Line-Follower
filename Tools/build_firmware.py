"""Build the Blue Pill firmware with GNU Arm and the official local STM32CubeF1 package."""
import argparse
from pathlib import Path
import shutil
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("cube_f1", type=Path, help="STM32Cube_FW_F1_V1.8.5 directory containing Drivers")
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
build = root / "build"
drivers = args.cube_f1.resolve() / "Drivers"
hal = drivers / "STM32F1xx_HAL_Driver"
cmsis = drivers / "CMSIS"
device = cmsis / "Device/ST/STM32F1xx"
templates = device / "Source/Templates"
startup = templates / "gcc/startup_stm32f103xb.s"
if not startup.is_file() or not (hal/"Inc/stm32f1xx_hal_conf_template.h").is_file():
    parser.error("Check the CubeF1 package path and its Drivers directory.")
cc = shutil.which("arm-none-eabi-gcc")
if not cc:
    parser.error("Add the STM32CubeIDE GNU Arm toolchain bin directory to PATH.")
build.mkdir(exist_ok=True)
shutil.copyfile(hal/"Inc/stm32f1xx_hal_conf_template.h", build/"stm32f1xx_hal_conf.h")
includes = [root/"Core/Inc", build, hal/"Inc", cmsis/"Include", device/"Include"]
modules = ["hal", "hal_adc", "hal_adc_ex", "hal_cortex", "hal_dma", "hal_flash",
           "hal_flash_ex", "hal_gpio", "hal_gpio_ex", "hal_pwr", "hal_rcc",
           "hal_rcc_ex", "hal_tim", "hal_tim_ex"]
# GPIOEx can be header-only in this HAL family.
sources = [hal/"Src"/("stm32f1xx_"+m+".c") for m in modules if
           m != "hal_gpio_ex" or (hal/"Src/stm32f1xx_hal_gpio_ex.c").is_file()]
elf = build/"line_follower.elf"
subprocess.run([cc, "-mcpu=cortex-m3", "-mthumb", "-std=gnu11", "-Os", "-g3",
                "-ffunction-sections", "-fdata-sections", "-DSTM32F103xB", "-DUSE_HAL_DRIVER",
                *["-I"+str(p) for p in includes], *map(str, sorted((root/"Core/Src").glob("*.c"))),
                *map(str,sources), str(templates/"system_stm32f1xx.c"), str(startup),
                "-T"+str(root/"Tools/STM32F103C8.ld"), "--specs=nano.specs", "--specs=nosys.specs",
                "-Wl,--gc-sections", "-o", str(elf)], check=True)
print(elf)
