/// \file UnitDefs_Base.hh SI units system definitions
#ifndef UNITDEFS_BASE_HH
#define UNITDEFS_BASE_HH

namespace Units {

    // base units {m, kg, s, A, K, mol, cd}
    const Unitful meter(1., dimensions_t{{1,0,0,0,0,0,0}}); //< meter
    const Unitful kg(1.,    dimensions_t{{0,1,0,0,0,0,0}}); //< kilogram
    const Unitful second(1.,dimensions_t{{0,0,1,0,0,0,0}}); //< second
    const Unitful ampere(1.,dimensions_t{{0,0,0,1,0,0,0}}); //< ampere
    const Unitful kelvin(1.,dimensions_t{{0,0,0,0,1,0,0}}); //< Kelvin
    const Unitful mol(1.,   dimensions_t{{0,0,0,0,0,1,0}}); //< mol
    const Unitful candela(1,dimensions_t{{0,0,0,0,0,0,1}}); //< candela

    // unitless prefixes
    const double yotta(1e24);       //< yotta (\f$10^{24}\f$) prefix
    const double zetta(1e21);       //< zetta (\f$10^{21}\f$) prefix
    const double exa(1e18);         //< exa (\f$10^{18}\f$) prefix
    const double tera(1e15);        //< tera (\f$10^{15}\f$) prefix
    const double peta(1e12);        //< peta (\f$10^{12}\f$) prefix
    const double giga(1e9);         //< giga (\f$10^{9}\f$) prefix
    const double mega(1e6);         //< mega (\f$10^{6}\f$) prefix
    const double kilo(1e3);         //< kilo (\f$10^{3}\f$) prefix
    const double hecto(1e2);        //< hecto (\f$10^{2}\f$) prefix
    const double deka(1e1);         //< deca (\f$10^{1}\f$) prefix
    const double deci(1e-1);        //< deci (\f$10^{-1}\f$) prefix
    const double centi(1e-2);       //< centi (\f$10^{-2}\f$) prefix
    const double milli(1e-3);       //< milli (\f$10^{-3}\f$) prefix
    const double micro(1e-6);       //< micro (\f$10^{-6}\f$) prefix
    const double nano(1e-9);        //< nano (\f$10^{-9}\f$) prefix
    const double pico(1e-12);       //< pico (\f$10^{-12}\f$) prefix
    const double femto(1e-15);      //< femto (\f$10^{-15}\f$) prefix
    const double atto(1e-18);       //< atto (\f$10^{-18}\f$) prefix
    const double zepto(1e-21);      //< zepto (\f$10^{-21}\f$) prefix
    const double yocto(1e-24);      //< yocto (\f$10^{-24}\f$) prefix

    const double kibi(1<<10);       //< kibi (\f$2^{10}\f$) prefix
    const double mebi = kibi*kibi;  //< mebi (\f$2^{20}\f$) prefix
    const double gibi = kibi*mebi;  //< gibi (\f$2^{30}\f$) prefix
    const double tebi = kibi*gibi;  //< tebi (\f$2^{40}\f$) prefix
    const double pebi = kibi*tebi;  //< pebi (\f$2^{50}\f$) prefix
    const double exbi = kibi*pebi;  //< exbi (\f$2^{60}\f$) prefix

    const Unitful cm(centi, meter);     //< centimeter
    const Unitful mm(milli, meter);     //< millimeter
    const Unitful micron(micro, meter); //< micron (micrometer)
    const Unitful angstrom(1e-10, meter);   //< angstrom ( \f$ 1\AA \equiv 10^{-10} m \f$ )
    const Unitful fermi(femto, meter);  //< fermi (femtometer)

    const Unitful inch(2.54, cm);       //< inch
    const Unitful mil(1e-3, inch);      //< mil (1/1000 inch)
    const Unitful digit(.75, inch);     //< digit (3/4 inch)
    const Unitful finger(.875, inch);   //< finger (7/8 inch)
    const Unitful palm(4, digit);       //< palm (4 digits; 3 inches)
    const Unitful shaftment(2, palm);   //< shaftment (2 palms)
    const Unitful hand(4, inch);        //< hand (4 inches)
    const Unitful span(9, inch);        //< span (half cubit; 9 inches)
    const Unitful foot(12, inch);       //< foot (12 inches)
    const Unitful cubit(18, inch);      //< cubit (18 inches)
    const Unitful ell(45, inch);        //< English ell (45 inches)
    const Unitful yard(3, foot);        //< yard (3 feet; 2 cubits)
    const Unitful fathom(2, yard);      //< fathom (6 feet; 2 yards)
    const Unitful rope(20, foot);       //< rope (20 feet)
    const Unitful chain(11, fathom);    //< surveyor's chain (66 feet)
    const Unitful rod(.25, chain);      //< rod (quarter chain)
    const Unitful link(.01, chain);     //< link (1/100 chain)
    const Unitful furlong(10, chain);   //< furlong (660 feet; 1/8 mile)
    const Unitful mile(8, furlong);     //< international mile (5280 feet; 8 furlongs)

    const Unitful ftUS(1200./3937., meter); //< US survey foot
    const Unitful mileUS(5280, ftUS);       //< US statute (survey) mile

    const Unitful pt(1./72, inch);      //< desk-top publishing (DTP) point
    const Unitful pica(12, pt);         //< typesetting pica (12 points)
    const Unitful twip(.05, pt);        //< twip (twentieth point)

    const Unitful nmi(1852, meter);     //< international nautical mile
    const Unitful cable(.1, nmi);       //< international cable (1/10 nautical mile)

    // obsolete Russian units of measure
    const Unitful diyum(1,inch);        //< Russian diyum (inch)
    const Unitful liniya(.1, diyum);    //< Russian liniya
    const Unitful tochka(.01, liniya);  //< Russian tochka
    const Unitful vershok(7./4, diyum); //< Russian vershok
    const Unitful pyad(4, vershok);     //< Russian pyad
    const Unitful fut(12, diyum);       //< Russian fut (foot)
    const Unitful arshin(28, diyum);    //< Russian arshin
    const Unitful sazhen(7, fut);       //< Russian sazhen
    const Unitful versta(500, sazhen);  //< Russian versta
    const Unitful milya(7, versta);     //< Russian milya

    const Unitful au(149597870691., meter);     //< astronomical unit, \f$\pm\f$30m
    const Unitful parsec(19176075967.937, mile);//< parsec
    const Unitful kpc(kilo, parsec);            //< kiloparsec
    const Unitful Mpc(mega, parsec);            //< megaparsec

    const Unitful gram(1e-3, kg);               //< gram
    const Unitful milligram(milli, gram);       //< milligram
    const Unitful carat(200, milligram);        //< carat (200gm)
    const Unitful point(.01, carat);            //< point (1/100 carat)
    const Unitful grain(64.79891, milligram);   //< grain
    const Unitful lb(7000, grain);              //< avoirdupois pound mass (7000 grain)
    const Unitful oz(1./16, lb);                //< ounce (1/16 lb)
    const Unitful dram(1./16, oz);              //< dram (1/16 oz)
    const Unitful stone(14, lb);                //< stone (14 lb)
    const Unitful hundredweight(100, lb);       //< hundredweight (100 lb)

    const Unitful pennyweight(24, grain);       //< pennyweight (1 dwt; 24 grain)
    const Unitful ozTroy(20, pennyweight);      //< troy ounce (20 dwt)
    const Unitful lbTroy(12, ozTroy);           //< troy pound (12 troy oz)

    const Unitful scrupleApoth(20, grain);      //< Apothecaries' scruple (1/3 dram; 20 grain)
    const Unitful dramApoth(3, scrupleApoth);   //< Apothecaries' dram (1/8 oz; 60 grain)
    const Unitful ozApoth(8, dramApoth);        //< Apothecaries' ounce (1/12 lb; 480 grain)
    const Unitful lbApoth(12, ozApoth);         //< Apothecaries' pound (5760 grain)

    const Unitful minute(60, second);           //< minute
    const Unitful hour(60, minute);             //< hour
    const Unitful day(24, hour);                //< day
    const Unitful week(7, day);                 //< week
    const Unitful fortnight(2, week);           //< fortnight
    const Unitful year(365, day);               //< calendar year
    const Unitful yearSolar(365.242199, day);   //< solar year
}

#endif
