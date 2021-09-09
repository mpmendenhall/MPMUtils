#ifndef UNITDEFS_MECHANICS_HH
#define UNITDEFS_MECHANICS_HH 1

namespace SI {

	/// Frequency in SI system
	class SIFrequency: public Unitful {
	public:
		/// constructor
		SIFrequency(double t = 0): Unitful(t,second.inverse()) {}
		/// re-casting constructor
		SIFrequency(const unit7_t& u): Unitful(1,u) { assertConsistent(SIFrequency()); }
	};
	const SIFrequency hertz = second.inverse();				//< hertz


	/// Speed in SI system
	class SISpeed: public Unitful {
	public:
		/// constructor
		SISpeed(double t = 0): Unitful(t,meter/second) {}
		/// re-casting constructor
		SISpeed(const unit7_t& u): Unitful(1,u) { assertConsistent(SISpeed()); }
	};
	SISpeed c = meter/second*299792458;	//< speed of light
	SISpeed knot = nmi/hour;			//< knot (nautical miles per hour)
	Unitful lightyear = c*yearSolar;	//< lightyear
	Unitful lightsecond = c*second;	//< lightsecond


	/// Acceleration in SI system
	class SIAcceleration: public Unitful {
	public:
		/// constructor
		SIAcceleration(double t = 0): Unitful(t,meter/second/second) {}
		/// re-casting constructor
		SIAcceleration(const unit7_t& u): Unitful(1,u) { assertConsistent(SIAcceleration()); }
	};
	SIAcceleration g = meter/second/second*9.80665;	//< acceleration due to gravity at sea level


	/// Area in SI system
	class SIArea: public Unitful {
	public:
		/// constructor
		SIArea(double t = 0): Unitful(t,meter*meter) {}
		/// re-casting constructor
		SIArea(const unit7_t& u): Unitful(1,u) { assertConsistent(SIArea()); }
	};
	const SIArea are = meter*meter*100;			//< are (square decameter)
	const SIArea stremma = meter*meter*1000;	//< stremma (1000 square meters)
	const SIArea hectare = are*100;				//< hectare
	const SIArea barn = fermi*fermi*100;		//< barn (100 square fermis)

	const SIArea section = mile*mile;			//< section (square mile)
	const SIArea acre = furlong*chain;			//< acre (1/640 square mile)
	const SIArea rood = acre/4;					//< rood (quarter acre)
	const SIArea yardland = acre*30;			//< yardland (30 acres)
	const SIArea hide = acre*100;				//< hide (100 acres)
	const SIArea barony = acre*4000;			//< barony (4000 acres)
	const SIArea board = inch*foot;				//< board (inch by foot)
	const SIArea cord = board*192;				//< cord (192 boards)

	const SIArea township = mileUS*mileUS*36;	//< survey township (36 square survey miles)


	/// Volume in SI system
	class SIVolume: public Unitful {
	public:
		/// constructor
		SIVolume(double t = 0): Unitful(t,meter*meter*meter) {}
		/// re-casting constructor
		SIVolume(const unit7_t& u): Unitful(1,u) { assertConsistent(SIVolume()); }
	};
	const SIVolume ml = cm*cm*cm;	//< milliliter
	const SIVolume liter = ml*1000;	//< liter
	const SIVolume inch3 = inch*inch*inch; //< cubic inch
	const SIVolume foot3 = foot*foot*foot; //< cubic foot

	const SIVolume floz = ml*29.57353;	//< fluid ounce
	const SIVolume tbsp = floz/2;		//< tablespoon	(1/2 fluid oz)
	const SIVolume tsp = tbsp/3;		//< teaspoon	(1/6 fluid oz)
	const SIVolume fldram = floz/8;		//< fluid dram (1/8 fluid oz)
	const SIVolume minim = fldram/60;	//< minim (1/60 fluid dram)
	const SIVolume gill = floz*4;		//< gill (4 fluid oz; 1/2 cup)
	const SIVolume cup = floz*8;		//< cup (8 fluid oz; 2 gill)
	const SIVolume pint = cup*2;		//< pint (2 cups)
	const SIVolume quart = pint*2;		//< quart (2 pints)
	const SIVolume gallon = quart*4;	//< gallon (4 quarts)
	const SIVolume barrel = gallon*42;	//< oil barrel (42 gallons)
	const SIVolume beerbbl = gallon*31;	//< beer barrel (31 gallons)
	const SIVolume hogshead = barrel*2;	//< hogshead (2 barrels) ?

	const SIVolume dryPint = inch3*33.6003125;	//< dry pint
	const SIVolume dryQuart = dryPint*2;		//< dry quart
	const SIVolume dryGallon = dryQuart*4;		//< dry gallon
	const SIVolume peck = dryGallon*2;			//< peck (2 dry gallons; quarter bushel)
	const SIVolume kenning = peck*2;			//< kenning (2 pecks; half bushel)
	const SIVolume bushel = kenning*2;			//< bushel (4 pecks)


	/// Force in SI system
	class SIForce: public Unitful {
	public:
		/// constructor
		SIForce(double t = 0): Unitful(t,kg*meter/(second*second)) {}
		/// re-casting constructor
		SIForce(const unit7_t& u): Unitful(1,u) { assertConsistent(SIForce()); }
	};
	const SIForce newton = kg*meter/(second*second);	//< newton
	const SIForce dyne = newton*1e-5;					//< dyne (\f$ 10^{-5}\f$ Newtons)
	const SIForce lbf = lb*g;							//< pound-force
	const SIForce poundal = lb*foot/second/second;		//< poundal (pdl)
	const SIForce kgf = kg*g;							//< kilogram-force
	const SIMass slug = lbf/(foot/second/second);		//< slug mass

	/// Pressure in SI system
	class SIPressure: public Unitful {
	public:
		/// constructor
		SIPressure(double t = 0): Unitful(t,newton/(meter*meter)) {}
		/// re-casting constructor
		SIPressure(const unit7_t& u): Unitful(1,u) { assertConsistent(SIPressure()); }
	};
	const SIPressure pascal = newton/(meter*meter);		//< pascal
	const SIPressure bar = pascal*100000;				//< bar
	const SIPressure atm = pascal*101325;				//< atmosphere

	/// Energy in SI system
	class SIEnergy: public Unitful {
	public:
		/// constructor
		SIEnergy(double t = 0): Unitful(t,newton*meter) {}
		/// re-casting constructor
		SIEnergy(const unit7_t& u): Unitful(1,u) { assertConsistent(SIEnergy()); }
	};
	const SIEnergy joule = newton*meter;		//< joule
	const SIEnergy calorie = joule*4.184;		//< thermochemical calorie
	const SIEnergy cal15 = joule*4.1858;		//< 15 degree celcius calorie
	const SIEnergy calIT = joule*4.1868;		//< International Steam Table (1956) calorie
	const SIEnergy btu = (lb/gram*5.0/9.0)*calIT;	//< british thermal unit, IT version

	/// Power in SI system
	class SIPower: public Unitful {
	public:
		/// constructor
		SIPower(double t = 0): Unitful(t,joule/second) {}
		/// re-casting constructor
		SIPower(const unit7_t& u): Unitful(1,u) { assertConsistent(SIPower()); }
	};
	const SIPower watt = joule/second;				//< watt
	const SIPower hp = foot*lbf/second*550;			//< mechanical horsepower
	const SIPower metricHP = meter*kgf/second*75;	//< metric horsepower
	const SIPower electricalHP = watt*746;			//< electrical horsepower
	const SIPower boilerHP = btu/hour*33475;		//< boiler horsepower

}

#endif
