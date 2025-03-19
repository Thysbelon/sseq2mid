# sseq2mid

An updated fork of a [tool originally written by loveemu](https://github.com/loveemu/loveemu-lab/tree/master/nds/sseq2mid). Programmed to support almost all sseq commands; any sseq commands that do not have a midi equivalent are exported as either undefined midi CCs or text markers, so they will not be audible when listening to the midi, but they will be detected and converted when using my fork of [midi2sseq](https://github.com/Thysbelon/midi2sseq).

To listen to converted midi files, I recommend using [VGMTrans](https://github.com/vgmtrans/vgmtrans) to convert an NDS song's sound bank to an SF2.

To edit the midi files produced by this program, please use a midi editor that supports multiple tracks in midi files (type 1 midi files). This is needed so that text markers corresponding to sseq commands are placed on the correct tracks.   
When opening this program's midi files in a DAW such as Reaper, please choose to separate tracks, but do not separate channels.

This software has not been rigorously tested. If you encounter a bug, please open an issue in the issues tab and state the game and song with which you experienced the issue. I may not immediately respond to issues, but I always appreciate receiving them.

## List of Special Undefined Midi CC and Text Markers, and the sseq Commands they Convert to

Please refer to [sequence.md](https://github.com/Thysbelon/midi2sseq/blob/master/sequence.md) to learn what the sseq commands do.
- `Random:0x??,#,#` - `0xA0`
- `UseVar:0x??,var#` - `0xA1`
- `If:<other command's marker text>` - `0xA2`
- `Var:var#,<operation>,#` - `0xB0` to `0xBD` (except 0xB7)
- `CC14` - `0xC6` (sets priority, which determines if the game engine should interrupt this track when playing sound effects)
- `Tie:<on/off>` - `0xC8`
- `CC21` - `0xCB` (sets speed of vibrato/tremolo/autopan)
- `CC22` - `0xCC` (sets the LFO type to vibrato, tremolo or autopan)
- `CC3` - `0xCD`
- `CC75` / Sound Controller 6 - `0xD1`
- `CC76` / Sound Controller 7 - `0xD2`
- `PrintVar:var#` - `0xD6`
- `CC26` or `ModDelay:#` - `0xE0` (sets the delay before vibrato/tremolo/autopan starts when a note is played)
- `SweepPitch:#` - `0xE3`

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

