# CDImage (CD-R fork)

Fork of [arduinocelentano/cdimage](https://github.com/arduinocelentano/cdimage) with a tested **CD-R** preset, a headless track maker, and notes on measuring a new disc instead of guessing.

## Tested preset: Verbatim CD-R Extra Protection

| Disc | ATIP | 1st track length | Track delta | Start radius |
|---|---|---|---|---|
| Verbatim CD-R 52x 700 MB 80 min "Extra Protection" (CMC Magnetics, phthalocyanine dye) | 97m26s66f, lead-out 79:59:71 | **23347.02** | **1.3899902** | **25.2766** |

Burned in DAO at 10x with an LG GP60NS50 slim USB drive. The picture comes out straight across the whole disc. It is pale (the dye is light), so view it in a dark room with one small light held low and to the side. A light straight above only shows rainbows.

The preset is in the GUI's list. Other batches of the same product may be made in another factory; check the ATIP (`wodim dev=/dev/sr0 -atip`) matches before using these numbers.

## Headless track maker and burn script

```
mkdir build-cli && cd build-cli && qmake6 ../cdimage-cli.pro && make && cd ..
./build-cli/cdimage-cli picture.png track.raw 23347.02 1.3899902 25.2766
wodim -v dev=/dev/sr0 speed=10 -dao -audio track.raw
```

The picture is stretched to fill the whole disc: use a square image with the disc filling the frame (a ring from 25 mm to 58 mm, hole in the middle). `burn-picture.sh picture.png` does both steps for the preset above and checks a blank disc is in first (`DEV=/dev/srN` for another drive).

## Measuring a new disc (instead of guessing)

Guessing the two numbers is slow: they need 5-8 correct digits, and a wrong guess usually shows nothing at all. What worked:

1. **Measure the spiral with the drive itself** using [disc-probe](https://github.com/Ashley-QwQ/disc-probe). Burn its random-data disc (`python -m discprobe random --media cd`, then `cdrdao write` with the toc it writes), then run `python -m discprobe geo --media cd --drive /dev/sr0`. Use a read speed where the drive spins at a fixed rate (CAV); `timing` tells you. It prints `u = r0/b` and `v = p/b`.
2. **Convert to cdimage numbers** (24 audio bytes per 588 channel bits):
   - track delta = 2π · v · 24/588
   - 1st track length = 2π · u · 24/588 **+ track delta / 2**
   - start radius = r0 printed by `geo`

   The `+ track delta / 2` is needed because cdimage starts turn N at `tr0·N + dtr·N(N-1)/2` bytes, while a constant-pitch spiral needs `tr0·N + dtr·N²/2`. Without it the spokes twist by about 100° across the disc.
3. **Burn the spokes test image** and fix the last constant. Drive timing cannot see exactly where the data starts along the track. That shows up as straight spokes that miss the centre slightly (twist mostly near the hub). Raise or lower the 1st track length by about `twist_in_turns / 0.343` (measured between 27 and 55 mm). On the disc above this was +0.16 (the leftover twist turned the same way as the uncorrected twist).

That took 1 measuring disc and 2 spoke tests.

`test-pics/spokes.png` and `test-pics/rings.png` are test patterns. Rings show even with wrong numbers (check the dye gives contrast); spokes only show once the numbers are right.

### Calibration sweep (optional)

Set `SWEEP_LO` and `SWEEP_HI` (percent) to make the numbers drift from the inner to the outer edge, assuming both disc speed and track pitch change together: `tr = tr0·(1-x) + n·dtr·(1-2x)`. A `<track>.sweep.csv` lists which numbers each radius used. This assumption turned out wrong for the disc above, so prefer measuring.

---

# CDImage
## A tool for burning visible pictures on a compact disc surface

![alt text](https://github.com/arduinocelentano/cdimage/blob/main/demo.png)

## Project's Origins and Credits
I know of at least two successful attempts to implement a similar technique. One was accomplished about 15 years ago by [argon](https://www.instructables.com/Burning-visible-images-onto-CD-Rs-with-data-beta/) Instructables user. [Another attempt](http://undefer.narod.ru/cdpaint/index.html) was made by a user with nickname [unDEFER] (no English documentation unfortunately). These two projects inspired me some time ago. And in fact my coordinate conversion code is mostly based on [unDEFER]’s implementation. I also used geometric parameters of some compact discs from that project. I acknowledge and am grateful to these developers for their contributions.

I played with color shades and different compact discs with moderate success and created a GUI with visual preview mode. I tried to implement a user-friendly solution but finally abandoned the project in 2008 due to a problem of calibration for every particular brand and type of compact disc. Recently I found my old code and decided to share it. More as a tribute to the compact disc era. But maybe you will make some use of it. I fixed some obvious bugs, brushed the code up a little and ported it to modern Qt6. I considered porting to Python but I still need C++ because audio track generation takes some time even on modern hardware.

## Building
You'll need [Qt 6](https://www.qt.io/product/qt6) library to build it. Just run `qmake` and then `make`. Alternatively you could build the project with [Qt Creator](https://www.qt.io/product/development-tools) if you installed it.

## Windows Users
Since many Windows users experienced problems with building it or did not want to install Qt and all the stuff, I've made a [Windows binary build](https://github.com/arduinocelentano/cdimage/releases).
However I have not tested it thoroughly yet because I don't have a working CD recorder at hand right now.
I think I'll try to burn some discs with Windows tools later and note down the steps for Windows users.

## Before you start
If your compact disc is not mentioned in the track generating dialog, you'll probably fail. All the discs are slightly different geometrically. This difference does not matter for data storage but dramatically influences image calculation.
You could try to guess geometry of an unknown disc and input it manually, but you'll probably spoil a lot of discs before you get some results. Moreover, all your test discs should be perfectly identical or you should use the same CD-RW disc. Sometimes discs of the same model happen to have different parameters. Don't know why. After all, manufacturers have never cared since those discs were never supposed to be used this way.

## Usage
1. Click **Edit→Load image** and select an image file. I recommend that you use a high-contrast image. If you choose a full color image, it will be converted to grayscale. You can adjust and scale your image:

* **left mouse button** — move image;

* **double click** — center image;

* **mouse scroll wheel** — zoom image.

2. Click Edit→Create track and select the model of your compact disc. 

> If your disc is not in the list (which is likely), you may input geometry manually. However unknown disc calibration is neither easy nor quick procedure. If you still wish to give it a try, I recommend that you read the ["Red Book"](https://www.ecma-international.org/wp-content/uploads/ECMA-130_2nd_edition_june_1996.pdf) as well as  [Considering Calibration](#considering-calibration) section before you start. **If you know the geometry of some compact disc which is not in the list, let me know and I’ll include it into the distribution.**

Depending on your hardware, conversion will take some time. Finally you’ll get a huge Audio CD track. *Yes, about 800Mb, which is normal for **Audio CD**.*

3. You could use any software you like to burn it. For example:

`cdrecord -audio dev=<recorder_device> <generated_track>`

Remember that you should create an **Audio CD**! 

## Considering Calibration 
From the Mathematical point of view we have a sort of [multi-objective optimization](https://en.wikipedia.org/wiki/Multi-objective_optimization) problem. Bicriteria optimization, to be more precise. It means that two objective functions should be optimized simultaneously. If we define goal as getting a "neat image", we need an expert who is able to provide some feedback regarding image "quality". Which leads us to [interactive methods](https://en.wikipedia.org/wiki/Multi-objective_optimization#Solution).

The first and the most obvious idea is to select some series of equally spaced values for each criterium and burn a lot of discs with all possible combinations. If the space is narrow enough, you'll see something at some discs. Then narrow the range and repeat. A typical Computational Mathematics approach. A lot of iterations and time.

The number of iterations could be reduced if we gradually change criteria within one image from disc center to rim. Then one should look for areas where some distinctive fragments could be seen. The only implementation I know about was a part of [CD PAINT](http://undefer.narod.ru/cdpaint/index.html) project I mentioned. I thought it was gone, but finally found it. It was called `defcdparams`. Apparently CD PAINT project contributors used it to define geometry of four discs I know. However it is still a time consuming procedure. I think it was the main reason why CD PAINT project was abandoned.

At least for me it was the reason why I finally gave up. However I'd like to share some of my thoughts regarding possible improvements of calibration technique. The weakest component in the mentioned algorithm is the necessity of human expertise. How it could be automated?

1. If all the discs are geometrically different, then seek time delays would be different to. Because the same sector A is located at different angles for two different discs. So if we know "ideal" delays for a calibrated disc, we theoretically should be able to calibrate another one. But... These delays must be optical drive dependent. So this solution potentially would face some hardware issues.

2. I have not considered this option in 2008, but now we have more advanced image recognition algorithms and better cameras. So maybe some AI solution might be used instead of human expertise.

If you have other ideas, please share them.

## Further readings

[Probably the first implementation of similar technique](https://www.instructables.com/Burning-visible-images-onto-CD-Rs-with-data-beta/).

[The "Red Book"](https://www.ecma-international.org/wp-content/uploads/ECMA-130_2nd_edition_june_1996.pdf) (CD-ROM ECMA standard).

[Hackaday post](https://hackaday.io/project/186303-burning-pictures-on-a-compact-disc-surface) about this project.

[A project regarding optical drive reverse engineering](https://scanlime.org/2016/08/scanlime001-coastermelt-part-1/). Not related directly to this project, but might be useful to give you a clue to how it probably could be done with hardware approach. Thanks to [some guy](https://hackaday.com/2022/07/11/burn-pictures-on-a-cd-r-no-special-drive-needed/#comment-6491772) for suggestion.
