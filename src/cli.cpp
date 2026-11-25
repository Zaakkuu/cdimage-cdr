// Headless track maker. Same Converter as the GUI; the picture is stretched to fill
// the whole disc. Optional calibration sweep via SWEEP_LO / SWEEP_HI env (percent).
// usage: cdimage-cli <picture> <out-track> [tr0 dtr r0]
#include <QCoreApplication>
#include <QImage>
#include <cstdio>
#include "converter.h"

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);
	if (argc < 3) { fprintf(stderr, "usage: %s picture out-track [tr0 dtr r0]\n", argv[0]); return 1; }
	QImage img(argv[1]);
	if (img.isNull()) { fprintf(stderr, "cannot read %s\n", argv[1]); return 1; }
	img = img.scaled(3000, 3000, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_RGB32);
	double tr0 = argc > 3 ? atof(argv[3]) : 22951.52, dtr = argc > 4 ? atof(argv[4]) : 1.3865961, r0 = argc > 5 ? atof(argv[5]) : 24.5;
	Converter c(nullptr, tr0, dtr, r0);
	c.setMixColors(false);
	int last = -1;
	QObject::connect(&c, &Converter::progressChanged, [&](int p){ if (p/10 != last) { last = p/10; fprintf(stderr, "%d%%\n", p); } });
	return c.convert(img, argv[2]) ? 0 : 1;
}
