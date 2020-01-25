import sys
from PIL import Image
import anvil
import blockcolors

def main(region, outname):
    outimg = Image.new('RGBA', (16*16,16*16))

    for chz in range(16):
        for chx in range(16):
            try:
                chunk = anvil.Chunk.from_region(region, chx, chz)
            except:
                print('nonexisting chunk')
                for x in range(16):
                    for z in range(16):
                        outimg.putpixel((chx * 16 + x, chz * 16 + z), (0, 0, 0))
                continue

            for z in range(16):
                prevy = -1
                for x in range(16):
                    for y in reversed(range(256)):
                        b = chunk.get_block(x, y, z).id
                        if b == 'air' or b == 'cave_air' or b == 'void_air':
                            if y == 0:
                                outimg.putpixel((chx * 16 + x, chz * 16 + z), (0, 0, 0))
                            continue

                        # `b' is non-air block so we write out this block to image.
                        if b in blockcolors.colors:
                            color = blockcolors.colors[b]
                            color_list = [color[0], color[1], color[2]]
                            if prevy < 0 or prevy == y:
                                plus = 0
                            elif prevy < y:
                                plus = 30
                            else:
                                plus = -30
                            for ci in range(3):
                                color_list[ci] += plus
                                if color_list[ci] > 255:
                                    color_list[ci] = 255
                                elif color_list[ci] < 0:
                                    color_list[ci] = 0

                            outimg.putpixel((chx * 16 + x, chz * 16 + z), (color_list[0], color_list[1], color_list[2]))
                        else:
                            print(f'colors[\'{b}\'] = ???')
                            outimg.putpixel((chx * 16 + x, chz * 16 + z), (0, 0, 0))

                        prevy = y

                        break

    outimg.save(outname)

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('usage: top_view region_file outname')
        exit()
    else:
        main(sys.argv[1], sys.argv[2])
