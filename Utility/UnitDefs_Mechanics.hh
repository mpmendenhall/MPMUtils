/// \file UnitDefs_Mechanics.hh Mechanics units (mks based)

#ifndef UNITDEFS_MECHANICS_HH
#define UNITDEFS_MECHANICS_HH

namespace Units {
    // frequency
    const Unitful hertz = Unitful(1)/second;    //< hertz

    // velocity & friends
    Unitful c(299792458, meter/second);         //< speed of light
    Unitful knot = nmi/hour;                    //< knot (nautical miles per hour)
    Unitful lightyear = c*yearSolar;            //< lightyear
    Unitful lightsecond = c*second;             //< lightsecond

    // acceleration
    Unitful g(9.80665, meter/second/second);    //< acceleration due to gravity at sea level

    // area
    const Unitful are(100, meter*meter);        //< are (square decameter)
    const Unitful stremma(10, are);             //< stremma (1000 square meters)
    const Unitful hectare(100, are);            //< hectare
    const Unitful barn(100, fermi*fermi);       //< barn (100 square fermis)

    const Unitful section = mile*mile;          //< section (square mile)
    const Unitful acre = furlong*chain;         //< acre (1/640 square mile)
    const Unitful rood(.25, acre);              //< rood (quarter acre)
    const Unitful yardland(30, acre);           //< yardland (30 acres)
    const Unitful hide(100, acre);              //< hide (100 acres)
    const Unitful barony(4000, acre);           //< barony (4000 acres)
    const Unitful board = inch*foot;            //< board (inch by foot)
    const Unitful cord(192, board);             //< cord (192 boards)

    const Unitful township(36, mileUS*mileUS);  //< survey township (36 square survey miles)

    // volume
    const Unitful ml = cm*cm*cm;                //< milliliter
    const Unitful liter(1000, ml);              //< liter
    const Unitful inch3 = inch*inch*inch;       //< cubic inch
    const Unitful foot3 = foot*foot*foot;       //< cubic foot

    const Unitful floz(29.57353, ml);           //< fluid ounce
    const Unitful tbsp(.5, floz);               //< tablespoon    (1/2 fluid oz)
    const Unitful tsp(1./3, tbsp);              //< teaspoon    (1/6 fluid oz)
    const Unitful fldram(1/8., floz);           //< fluid dram (1/8 fluid oz)
    const Unitful minim(1/60., fldram);         //< minim (1/60 fluid dram)
    const Unitful gill(4, floz);                //< gill (4 fluid oz; 1/2 cup)
    const Unitful cup(8, floz);                 //< cup (8 fluid oz; 2 gill)
    const Unitful pint(2, cup);                 //< pint (2 cups)
    const Unitful quart(2, pint);               //< quart (2 pints)
    const Unitful gallon(4, quart);             //< gallon (4 quarts)
    const Unitful barrel(42, gallon);           //< oil barrel (42 gallons)
    const Unitful beerbbl(31, gallon);          //< beer barrel (31 gallons)
    const Unitful hogshead(2, barrel);          //< hogshead (2 barrels) ?

    const Unitful dryPint(33.6003125, inch3);   //< dry pint
    const Unitful dryQuart(2, dryPint);         //< dry quart
    const Unitful dryGallon(4, dryQuart);       //< dry gallon
    const Unitful peck(2, dryGallon);           //< peck (2 dry gallons; quarter bushel)
    const Unitful kenning(2, peck);             //< kenning (2 pecks; half bushel)
    const Unitful bushel(2, kenning);           //< bushel (4 pecks)

    // force
    const Unitful newton = kg*meter/(second*second);    //< newton
    const Unitful dyne(1e-5, newton);                   //< dyne (\f$ 10^{-5}\f$ Newtons)
    const Unitful lbf = lb*g;                           //< pound-force
    const Unitful poundal = lb*foot/second/second;      //< poundal (pdl)
    const Unitful kgf = kg*g;                           //< kilogram-force
    const Unitful slug = lbf/(foot/second/second);      //< slug mass

    // pressure
    const Unitful pascal = newton/(meter*meter);    //< pascal
    const Unitful bar(100000, pascal);              //< bar
    const Unitful atm(101325, pascal);              //< atmosphere

    // energy
    const Unitful joule = newton*meter;             //< joule
    const Unitful calorie(4.184, joule);            //< thermochemical calorie
    const Unitful cal15(4.1858, joule);             //< 15 degree celcius calorie
    const Unitful calIT(4.1868, joule);             //< International Steam Table (1956) calorie
    const Unitful btu(5./9, calIT*lb/gram);         //< british thermal unit, IT version

    // power
    const Unitful watt = joule/second;               //< watt
    const Unitful hp(550, foot*lbf/second);          //< mechanical horsepower
    const Unitful metricHP(75, meter*kgf/second);    //< metric horsepower
    const Unitful electricalHP(746, watt);           //< electrical horsepower
    const Unitful boilerHP(33475, btu/hour);         //< boiler horsepower
}

#endif
