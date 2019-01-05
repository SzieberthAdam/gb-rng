import sys
import pathlib
import struct


nibble_popcount = {
  0x0: 0,
  0x1: 1,
  0x2: 1,
  0x3: 2,
  0x4: 1,
  0x5: 2,
  0x6: 2,
  0x7: 3,
  0x8: 1,
  0x9: 2,
  0xA: 2,
  0xB: 3,
  0xC: 2,
  0xD: 3,
  0xE: 3,
  0xF: 4,
}

def chunks(l, n):
    """Yield successive n-sized chunks from l."""
    for i in range(0, len(l), n):
        yield l[i:i + n]


def hexstr(d):
  return " ".join("{:02X}".format(x) for x in d)


def get_popcount(data):
  r = 0
  for b in data:
    r += nibble_popcount[b & 0xF]
    r += nibble_popcount[b >> 4]
  return r


def get_pcpy(data):
  popcount = get_popcount(data)
  lobyte = struct.pack("<L", popcount)[0]
  return lobyte & 1


def get_seed(data, seedlen):
  result = bytearray()
  for bytedata in chunks(data, len(data)//seedlen):
    byte = 0
    for bitdata in  chunks(bytedata, len(bytedata)//8):
      pcpy = get_pcpy(bitdata)
      byte = (byte << 1) + pcpy
    result.append(byte)
  return bytes(result)


def rom_dumps():
  root = pathlib.Path(__file__).absolute().parent.parent
  dump = root / "dump"
  yield from dump.glob("gbrng-ram.*.dump")


def main():
  for i, dumpfname in enumerate(rom_dumps()):
    if 0 < i:
      print()
    print(dumpfname.stem)
    seedfname = dumpfname.with_suffix(".postseed")
    if not seedfname.is_file():
      continue
    with dumpfname.open("rb") as f:
      dump = f.read()
    with seedfname.open("rb") as f:
      seed = f.read()
    seed1 = get_seed(dump, seedlen=len(seed))
    if seed == seed1:
      print(f'[ OK ]    {hexstr(seed)}')
    else:
      print(f'[ FAIL ]  {hexstr(seed1)}) != ({hexstr(seed)}')


if __name__ == "__main__":
  main()
