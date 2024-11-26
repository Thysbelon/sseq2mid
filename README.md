# sseq2mid

An updated fork of a [tool originally written by loveemu](https://github.com/loveemu/loveemu-lab/tree/master/nds/sseq2mid). Programmed to support almost all sseq commands; any sseq commands that do not have a midi equivalent are exported as either undefined midi CCs or text markers, so they will not be audible when listening to the midi, but they will be detected and converted when using my fork of [midi2sseq](https://github.com/Thysbelon/midi2sseq).

To listen to converted midi files, I recommend using [VGMTrans](https://github.com/vgmtrans/vgmtrans) to convert an NDS song's sound bank to an SF2.

This software has not been rigorously tested. If you encounter a bug, please open an issue in the issues tab and state the game and song with which you experienced the issue. I may not immediately respond to issues, but I always appreciate receiving them.

## Compilation Notes (for Developers)

The `.sln` and `.vcxproj` files are untested.

I always compile this program with

```
gcc src/libsmfc.c src/libsmfcx.c src/sseq2mid.c -o sseq2mid
```

I may write a Makefile later.

## Credits
- [Loveemu](https://github.com/loveemu)
- [Gota](https://github.com/Gota7) for [sequence.md](https://github.com/Thysbelon/midi2sseq/blob/master/sequence.md)
