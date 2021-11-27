
import marlin
from SCons.Script import DefaultEnvironment
env = DefaultEnvironment()
board = env.BoardConfig()
if 'offset' in board.get("build").keys():
    marlin.relocate_vtab(board.get('build.offset'))

#arm-none-eabi-objcopy -O ihex *.elf *.hex
#arm-none-eabi-objcopy -O binary *.elf *.bin
def output_target():
    # tar_hex = "output/fmw_greeny_f446.hex"
    tar_bin = "output/fmw_greeny_f446.bin"
    # env.AddPostAction(
    #     "$BUILD_DIR/${PROGNAME}.elf",
    #     env.VerboseAction(" ".join([
    #         "$OBJCOPY", "-O", "ihex", "-R", ".eeprom",
    #         "$BUILD_DIR/${PROGNAME}.elf", tar_hex
    #     ]), "Building %s" % tar_hex)
    # )
    env.AddPostAction(
        "$BUILD_DIR/${PROGNAME}.elf",
        env.VerboseAction(" ".join([
            "$OBJCOPY", "-O", "binary", "-R", ".eeprom",
            "$BUILD_DIR/${PROGNAME}.elf", tar_bin
        ]), "Building %s" % tar_bin)
    )
output_target()
