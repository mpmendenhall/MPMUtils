/// \file UnitDefs_Mechanics.hh Mechanics units (mostly mks based)

#ifndef UNITDEFS_MECHANICS_HH
#define UNITDEFS_MECHANICS_HH

namespace Units {
    //-////////
    // velocity

    const Unitful knot = nmi/hour;                  //< knot (nautical miles per hour)

    //-/////
    // force

    const Unitful newton = kg*meter/second/second;  //< newton
    const Unitful dyne(1e-5, newton);               //< dyne (\f$ 10^{-5}\f$ Newtons)
    const Unitful lbf = lb*g;                       //< pound-force
    const Unitful poundal = lb*foot/second/second;  //< poundal (pdl)
    const Unitful kgf = kg*g;                       //< kilogram-force
    const Unitful slug = lbf/(foot/second/second);  //< slug mass


    //-////////
    // pressure

    const Unitful pascal = newton/(meter*meter);    //< pascal
    const Unitful bar(100000, pascal);              //< bar
    const Unitful atm(101325, pascal);              //< atmosphere


    //-//////
    // energy

    const Unitful joule = newton*meter;             //< joule
    const Unitful calorie(4.184, joule);            //< thermochemical calorie
    const Unitful cal15(4.1858, joule);             //< 15 degree celcius calorie
    const Unitful calIT(4.1868, joule);             //< International Steam Table (1956) calorie
    const Unitful btu(5./9, calIT*lb/gram);         //< British thermal unit, IT version


    //-/////
    // power

    const Unitful watt = joule/second;               //< watt
    const Unitful hp(550, foot*lbf/second);          //< mechanical horsepower
    const Unitful metricHP(75, meter*kgf/second);    //< metric horsepower
    const Unitful electricalHP(746, watt);           //< electrical horsepower
    const Unitful boilerHP(33475, btu/hour);         //< boiler horsepower
}

#endif
