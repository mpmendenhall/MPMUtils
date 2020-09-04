#!/usr/bin/python3

from SpectrumUtils import *
from SpectrumPlots import *

from pyx import *
from pyx.graph.style import symbol
from math import *

def ColorT_Response(sensors, l0):
    dat = []
    for T in [1500 + 250*i for i in range(31)][::-1]:
        I = bbody_norm_spectrum(T)
        sig = [fdot(I,s) for s in sensors]
        lmean = sum([l0[n]*s for n,s in enumerate(sig)])/sum(sig)
        print(T, lmean)
        dat.append((T,lmean))

    print([p[0] for p in dat])
    print([p[1] for p in dat])

    g = graph.graphxy(width=15,height=10,
                        x=graph.axis.lin(title="blackbody temperature [K]"),
                        y=graph.axis.lin(title="mean detected wavelength [nm]"))

    g.plot(graph.data.points(dat,x=1,y=2), [graph.style.line(),])
    return g


def AS726x_ColorT_Response():
    l0 = [650., 600., 570., 550., 500., 450.]
    sensors = [gaussbump_spectrum(l, 40/(2*sqrt(2*log(2)))) for l in l0]
    ColorT_Response(sensors, l0).writetofile("AS726x_colorT.pdf")

def AS7341_ColorT_Response():
    sensors = [load_specfile("AS7341_%i.txt"%i)[0] for i in range(8)]
    l0 = [415, 445, 480, 515, 555, 590, 630, 680]
    ColorT_Response(sensors,l0).writetofile("AS7341_colorT.pdf")

def AS7341_Normalization():
    # datasheet channel counts for warm white 2700K LED at 107.67 uW/cm^2
    # csens = [55., 110., 210., 390., 590., 840., 1350., 1070.]
    # TODO
    sensors = [load_specfile("AS7341_%i.txt"%i)[0] for i in range(8)]
    LED2700 = load_specfile("AS7341_LED_2700K.txt")[0]
    fd0 = fdot(LED2700,sensors[-2], 350, 800)
    for s in sensors:
        fd = fdot(LED2700, s, 350, 800)
        print(spectrum_integral(s), '\t', fd*1350/fd0)


def extract_AS7341():

    g = graph.graphxy(width=15,height=10,
                    x=graph.axis.lin(title="x", min=300, max=1100),
                    y=graph.axis.lin(title="y", min=0),
                    key=graph.key.key(pos="tr"))

    def remap(x):
        u0 = [145.25,  169.38]
        u1 = [529.63,  308.]
        x0 = [350.,    0.]
        x1 = [1050.,   1.]
        w = [x0[i] + (x1[i]-x0[i])*(x[i]-u0[i])/(u1[i]-u0[i]) for i in range(2)]
        return w


    txt = open("/home/michael/Documents/Programming/pi/AdafruitDoodads/Multispectral_AS7341_response.svg","r").read().split('d="')[1:]
    curves = []
    for t in txt:
        t = t[:t.find('"')].split()
        if len(t) < 2: continue

        for pts in extract_svg_path(t):
            print(pts[0])
            if not 160 < pts[0][1] < 309: continue

            if not curves or curves[-1][-1][0] > pts[0][0]:
                curves.append(pts)
            else: curves[-1] += pts

    for c in curves:
        print("----------------------------")
        for p in c: print("%g\t%g"%tuple(p))

    curves = [c for c in curves if len(c) > 50]
    allpts = sum(curves)
    curves = [[remap(p) for p in c] for c in curves]

    for n,c in enumerate(curves):
        f = open("SpectraDat/AS7341_%i.txt"%n, 'w')
        f.write('\n'.join(["%g\t%g"%tuple(p) for p in c]))

    g.plot([graph.data.points(c, x=1, y=2, title="%i"%n) for n,c in enumerate(curves)], [graph.style.line([color.gradient.Rainbow]),])

    print(min([p[0] for p in allpts]), max([p[0] for p in allpts]))
    print(min([p[1] for p in allpts]), max([p[1] for p in allpts]))


    g.writetofile("AS7341_sens.pdf")

def extract_AS7341_cal():

    g = graph.graphxy(width=15,height=10,
                    x=graph.axis.lin(title="x"),#, min=300, max=1100),
                    y=graph.axis.lin(title="y"),#, min=0),
                    key=graph.key.key(pos="tr"))

    def remap(x):
        u0 = [183.1,   330.89]
        u1 = [661.89,  588.39]
        x0 = [350.,    0.]
        x1 = [1010.,   1.]
        w = [x0[i] + (x1[i]-x0[i])*(x[i]-u0[i])/(u1[i]-u0[i]) for i in range(2)]
        return w

    txt = open("/home/michael/Documents/Programming/pi/AdafruitDoodads/Multispectral_AS7341_calsrc.svg","r").read().split('d="')[1:]
    curves = []
    for t in txt:
        t = t[:t.find('"')].split()
        if len(t) < 2: continue

        for pts in extract_svg_path(t):
            print(pts[0])
            #if not 160 < pts[0][1] < 309: continue

            if not curves or curves[-1][-1][0] > pts[0][0]:
                curves.append(pts)
            else: curves[-1] += pts

    for c in curves:
        print("----------------------------")
        print(len(c))
        #for p in c: print("%g\t%g"%tuple(p))

    curves = [c for c in curves if len(c) > 110]
    allpts = []
    for c in curves: allpts += c
    curves = [[remap(p) for p in c] for c in curves]

    cnames = ["LED_420","LED_940","LED_2700K"]
    for n,c in enumerate(curves):
        f = open("SpectraDat/AS7341_%s.txt"%cnames[n], 'w')
        f.write('\n'.join(["%g\t%g"%tuple(p) for p in c]))

    g.plot([graph.data.points(c, x=1, y=2, title="%i"%n) for n,c in enumerate(curves)], [graph.style.line([color.gradient.Rainbow]),])

    print(min([p[0] for p in allpts]), max([p[0] for p in allpts]))
    print(min([p[1] for p in allpts]), max([p[1] for p in allpts]))


    g.writetofile("AS7341_cal.pdf")


#extract_AS7341()
#extract_AS7341_cal()

#AS726x_Response()
AS7341_ColorT_Response()
