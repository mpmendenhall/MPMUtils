/// @file UnitDefs_Base.hh SI base units, prefixes

#ifndef UNITDEFS_BASE_HH
#define UNITDEFS_BASE_HH

#include "UnitDefs.hh"

namespace Units {

    //-//////////
    // base units {m, kg, s, A, K, mol, cd}
    //  start with 12 so sqrt() etc. meaningful

    const Unitful meter(1., dimensions_t{{12,0,0,0,0,0,0}});    ///< meter
    const Unitful kg(1.,    dimensions_t{{0,12,0,0,0,0,0}});    ///< kilogram
    const Unitful second(1.,dimensions_t{{0,0,12,0,0,0,0}});    ///< second
    const Unitful ampere(1.,dimensions_t{{0,0,0,12,0,0,0}});    ///< ampere
    const Unitful kelvin(1.,dimensions_t{{0,0,0,0,12,0,0}});    ///< Kelvin
    const Unitful mol(1.,   dimensions_t{{0,0,0,0,0,12,0}});    ///< mol
    const Unitful candela(1,dimensions_t{{0,0,0,0,0,0,12}});    ///< candela


    //-////////////////////
    // unitless SI prefixes

    const double yotta = 1e24;      ///< yotta (\f$10^{24}\f$) prefix
    const double zetta = 1e21;      ///< zetta (\f$10^{21}\f$) prefix
    const double exa   = 1e18;      ///< exa (\f$10^{18}\f$) prefix
    const double tera  = 1e15;      ///< tera (\f$10^{15}\f$) prefix
    const double peta  = 1e12;      ///< peta (\f$10^{12}\f$) prefix
    const double giga  = 1e9;       ///< giga (\f$10^{9}\f$) prefix
    const double mega  = 1e6;       ///< mega (\f$10^{6}\f$) prefix
    const double kilo  = 1000;      ///< kilo (\f$10^{3}\f$) prefix
    const double hecto = 100;       ///< hecto (\f$10^{2}\f$) prefix
    const double deka  = 10;        ///< deca (\f$10^{1}\f$) prefix
    const double deci  = 0.1;       ///< deci (\f$10^{-1}\f$) prefix
    const double centi = 0.01;      ///< centi (\f$10^{-2}\f$) prefix
    const double milli = 1e-3;      ///< milli (\f$10^{-3}\f$) prefix
    const double micro = 1e-6;      ///< micro (\f$10^{-6}\f$) prefix
    const double nano  = 1e-9;      ///< nano (\f$10^{-9}\f$) prefix
    const double pico  = 1e-12;     ///< pico (\f$10^{-12}\f$) prefix
    const double femto = 1e-15;     ///< femto (\f$10^{-15}\f$) prefix
    const double atto  = 1e-18;     ///< atto (\f$10^{-18}\f$) prefix
    const double zepto = 1e-21;     ///< zepto (\f$10^{-21}\f$) prefix
    const double yocto = 1e-24;     ///< yocto (\f$10^{-24}\f$) prefix
    // powers-of-1024 prefixes
    const double kibi = 1<<10;      ///< kibi (\f$2^{10}\f$) prefix
    const double mebi = kibi*kibi;  ///< mebi (\f$2^{20}\f$) prefix
    const double gibi = kibi*mebi;  ///< gibi (\f$2^{30}\f$) prefix
    const double tebi = kibi*gibi;  ///< tebi (\f$2^{40}\f$) prefix
    const double pebi = kibi*tebi;  ///< pebi (\f$2^{50}\f$) prefix
    const double exbi = kibi*pebi;  ///< exbi (\f$2^{60}\f$) prefix


    //-/////////////////////
    // fundamental constants

    const Unitful c(299792458, meter/second);       ///< speed of light
    const Unitful g(9.80665, meter/second/second);  ///< acceleration due to gravity at sea level


    //-////
    // time

    const Unitful minute(60, second);           ///< minute
    const Unitful hour(60, minute);             ///< hour
    const Unitful day(24, hour);                ///< day
    const Unitful week(7, day);                 ///< week
    const Unitful fortnight(2, week);           ///< fortnight
    const Unitful year(365, day);               ///< calendar year
    const Unitful yearSolar(365.242199, day);   ///< solar year


    //-/////////
    // frequency

    const Unitful hertz = Unitful(1)/second;    ///< hertz


    //-//////
    // length

    const Unitful cm(centi, meter);     ///< centimeter
    const Unitful mm(milli, meter);     ///< millimeter
    const Unitful micron(micro, meter); ///< micron (micrometer)
    const Unitful angstrom(1e-10, meter);   ///< angstrom ( \f$ 1\AA \equiv 10^{-10} m \f$ )
    const Unitful fermi(femto, meter);  ///< fermi (femtometer)

    const Unitful inch(2.54, cm);       ///< inch
    const Unitful mil(1e-3, inch);      ///< mil (1/1000 inch)
    const Unitful digit(.75, inch);     ///< digit (3/4 inch)
    const Unitful finger(.875, inch);   ///< finger (7/8 inch)
    const Unitful palm(4, digit);       ///< palm (4 digits; 3 inches)
    const Unitful hand(4, inch);        ///< hand (4 inches)
    const Unitful shaftment(2, palm);   ///< shaftment (2 palms, 6 inches)
    const Unitful span(9, inch);        ///< span (half cubit; 9 inches)
    const Unitful foot(12, inch);       ///< foot (12 inches)
    const Unitful cubit(18, inch);      ///< cubit (18 inches, 2 span, half yard)
    const Unitful yard(3, foot);        ///< yard (3 feet; 2 cubits)
    const Unitful ell(45, inch);        ///< English ell (45 inches, 1/1408 mile, yard + span)
    const Unitful fathom(2, yard);      ///< fathom (6 feet; 2 yards; chain/11)
    const Unitful rope(20, foot);       ///< rope (20 feet)
    const Unitful chain(11, fathom);    ///< surveyor's chain (66 feet, furlong/10, mile/80)
    const Unitful rod = chain/4;        ///< rod (quarter chain)
    const Unitful link(.01, chain);     ///< link (1/100 chain)
    const Unitful furlong(10, chain);   ///< furlong (660 feet; 10 chain; 1/8 mile)
    const Unitful mile(8, furlong);     ///< international mile (5280 = 32*3*5*11 feet; 1760 yard; 1408 ell; 80 chain; 8 furlongs)

    const Unitful ftUS(1200./3937., meter); ///< US survey foot
    const Unitful mileUS(5280, ftUS);       ///< US statute (survey) mile

    // typography lengths
    const Unitful pt = inch/72;         ///< desk-top publishing (DTP) point
    const Unitful pica(12, pt);         ///< typesetting pica (12 points)
    const Unitful twip = pt/20;         ///< twip (twentieth point)

    // nautical lengths
    const Unitful nmi(1852, meter);     ///< international nautical mile
    const Unitful cable(.1, nmi);       ///< international cable (1/10 nautical mile)

    // astronomy lengths
    const Unitful au(149597870691., meter);     ///< astronomical unit, \f$\pm\f$30m
    const Unitful parsec(19176075967.937, mile);//< parsec
    const Unitful kpc(kilo, parsec);            ///< kiloparsec
    const Unitful Mpc(mega, parsec);            ///< megaparsec
    const Unitful lightyear = c*yearSolar;      ///< lightyear
    const Unitful lightsecond = c*second;       ///< lightsecond

    // obsolete Russian lengths
    const Unitful diyum(1,inch);        ///< Russian diyum (inch)
    const Unitful liniya(.1, diyum);    ///< Russian liniya
    const Unitful tochka(.01, liniya);  ///< Russian tochka
    const Unitful vershok(7./4, diyum); ///< Russian vershok
    const Unitful pyad(4, vershok);     ///< Russian pyad
    const Unitful fut(12, diyum);       ///< Russian fut (foot)
    const Unitful arshin(28, diyum);    ///< Russian arshin
    const Unitful sazhen(7, fut);       ///< Russian sazhen
    const Unitful versta(500, sazhen);  ///< Russian versta
    const Unitful milya(7, versta);     ///< Russian milya


    //-////
    // area

    const Unitful are(100, meter*meter);        ///< are (square decameter)
    const Unitful stremma(10, are);             ///< stremma (1000 square meters)
    const Unitful hectare(100, are);            ///< hectare (100 are)

    const Unitful barn(100, fermi*fermi);       ///< barn (100 square fermis)

    const Unitful section = mile*mile;          ///< section (square mile)
    const Unitful acre = furlong*chain;         ///< acre (1/640 square mile)
    const Unitful rood = acre/4;                ///< rood (quarter acre)
    const Unitful yardland(30, acre);           ///< yardland (30 acres)
    const Unitful hide(100, acre);              ///< hide (100 acres)
    const Unitful barony(4000, acre);           ///< barony (4000 acres)
    const Unitful board = inch*foot;            ///< board (inch by foot)
    const Unitful cord(192, board);             ///< cord (192 boards)

    const Unitful township(36, mileUS*mileUS);  ///< survey township (36 square survey miles)


    //-//////
    // volume

    const Unitful ml = cm*cm*cm;                ///< milliliter
    const Unitful liter(1000, ml);              ///< liter
    const Unitful inch3 = inch*inch*inch;       ///< cubic inch
    const Unitful foot3 = foot*foot*foot;       ///< cubic foot

    const Unitful floz(29.57353, ml);           ///< fluid ounce
    const Unitful tbsp(.5, floz);               ///< tablespoon (1/2 fluid oz)
    const Unitful tsp(1./3, tbsp);              ///< teaspoon (1/6 fluid oz)
    const Unitful fldram(1/8., floz);           ///< fluid dram (1/8 fluid oz)
    const Unitful minim(1/60., fldram);         ///< minim (1/60 fluid dram)
    const Unitful gill(4, floz);                ///< gill (4 fluid oz; 1/2 cup)
    const Unitful cup(8, floz);                 ///< cup (8 fluid oz; 2 gill)
    const Unitful pint(2, cup);                 ///< pint (2 cups)
    const Unitful quart(2, pint);               ///< quart (2 pints)
    const Unitful gallon(4, quart);             ///< gallon (4 quarts)
    const Unitful barrel(42, gallon);           ///< oil barrel (42 gallons)
    const Unitful beerbbl(31, gallon);          ///< beer barrel (31 gallons)
    const Unitful hogshead(2, barrel);          ///< hogshead (2 barrels) ?

    const Unitful dryPint(33.6003125, inch3);   ///< dry pint
    const Unitful dryQuart(2, dryPint);         ///< dry quart
    const Unitful dryGallon(4, dryQuart);       ///< dry gallon
    const Unitful peck(2, dryGallon);           ///< peck (2 dry gallons; quarter bushel)
    const Unitful kenning(2, peck);             ///< kenning (2 pecks; half bushel)
    const Unitful bushel(2, kenning);           ///< bushel (4 pecks)


    //-/////
    // mass

    const Unitful gram(1e-3, kg);               ///< gram
    const Unitful milligram(milli, gram);       ///< milligram
    const Unitful carat(200, milligram);        ///< carat (200gm)
    const Unitful point(.01, carat);            ///< point (1/100 carat)

    const Unitful grain(64.79891, milligram);   ///< grain
    const Unitful lb(7000, grain);              ///< avoirdupois pound mass (7000 grain)
    const Unitful oz = lb/16;                   ///< ounce (1/16 lb)
    const Unitful dram = oz/16;                 ///< dram (1/16 oz)
    const Unitful stone(14, lb);                ///< stone (14 lb)
    const Unitful hundredweight(100, lb);       ///< hundredweight (100 lb)
    const Unitful pennyweight(24, grain);       ///< pennyweight (1 dwt; 24 grain)

    const Unitful ozTroy(20, pennyweight);      ///< troy ounce (20 dwt)
    const Unitful lbTroy(12, ozTroy);           ///< troy pound (12 troy oz)

    const Unitful scrupleApoth(20, grain);      ///< Apothecaries' scruple (1/3 dram; 20 grain)
    const Unitful dramApoth(3, scrupleApoth);   ///< Apothecaries' dram (1/8 oz; 60 grain)
    const Unitful ozApoth(8, dramApoth);        ///< Apothecaries' ounce (1/12 lb; 480 grain)
    const Unitful lbApoth(12, ozApoth);         ///< Apothecaries' pound (5760 grain)

}

#endif
