import sys
import concurrent.futures

from PIL import Image
import anvil
import blockcolors

def generate_256(*args):
    region = args[0]
    region_x = args[1]
    region_z = args[2]
    offx = args[3]
    offz = args[4]
    verbose = args[5]
    if verbose:
        print('generating chunk: ' + str(offx) + ', ' + str(offz))

    outimg = Image.new('RGBA', (16*16,16*16))
    outname = str(region_x * 2 + offx) + ',' + str(region_z * 2 + offz) + '.png'
    for chz in range(16):
        for chx in range(16):
            try:
               chunk = anvil.Chunk.from_region(region, offx * 16 + chx, offz * 16 + chz)
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

def main(region_dir, region_x, region_z, nproc, verbose):
    if verbose:
        print('loading region')

    with concurrent.futures.ProcessPoolExecutor(max_workers=nproc) as executor:
        region = anvil.Region.from_file(region_dir + '/r.' + str(region_x) + '.' + str(region_z) + '.mca')
        for offx in range(2):
            for offz in range(2):
                executor.submit(generate_256, region, region_x, region_z, offx, offz, verbose)

def print_usage():
    print('usage: generate.py [--proc n_procs] [--verbose] region_dir region_x region_z')

if __name__ == '__main__':
    nproc = 2
    verbose = False
    normal_args = []

    i = 1
    while i < len(sys.argv):
        if sys.argv[i] == '--procs':
            i += 1
            if i < len(sys.argv):
                nproc = int(sys.argv[i + 1])
            else:
                print_usage()
                sys.exit(1)
        elif sys.argv[i] == '--verbose':
            verbose = True
        elif sys.argv[i] == '--help':
            print_usage()
            sys.exit(0)
        else:
            normal_args.append(sys.argv[i])

        i += 1

    if len(normal_args) < 2:
        print_usage()
        sys.exit(1)

    main(normal_args[0], int(normal_args[1]), int(normal_args[2]), nproc, verbose)
