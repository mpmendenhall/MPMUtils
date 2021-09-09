#ifndef UNITDEFS_EM_HH
#define UNITDEFS_EM_HH 1

namespace SI {

	/// Charge in SI system
	class SICharge: public Unitful {
	public:
		/// constructor
		SICharge(double t = 0): Unitful(t,ampere*second) {}
		/// re-casting constructor
		SICharge(const unit7_t& u): Unitful(1,u) { assertConsistent(SICharge()); }
	};
	const SICharge coulomb = ampere*second;		//< coulomb


	/// Electric potential in SI system
	class SIPotential: public Unitful {
	public:
		/// constructor
		SIPotential(double t = 0): Unitful(t,joule/coulomb) {}
		/// re-casting constructor
		SIPotential(const unit7_t& u): Unitful(1,u) { assertConsistent(SIPotential()); }
	};
	const SIPotential volt = joule/coulomb;		//< volt

	/// Capacitance in SI system
	class SICapacitance: public Unitful {
	public:
		/// constructor
		SICapacitance(double t = 0): Unitful(t,coulomb/volt) {}
		/// re-casting constructor
		SICapacitance(const unit7_t& u): Unitful(1,u) { assertConsistent(SICapacitance()); }
	};
	const SICapacitance farad = coulomb/volt;		//< farad


	/// Resistance in SI system
	class SIResistance: public Unitful {
	public:
		/// constructor
		SIResistance(double t = 0): Unitful(t,volt/ampere) {}
		/// re-casting constructor
		SIResistance(const unit7_t& u): Unitful(1,u) { assertConsistent(SIResistance()); }
	};
	const SIResistance ohm = volt/ampere;		//< ohm


	/// Conductance in SI system
	class SIConductance: public Unitful {
	public:
		/// constructor
		SIConductance(double t = 0): Unitful(t,ampere/volt) {}
		/// re-casting constructor
		SIConductance(const unit7_t& u): Unitful(1,u) { assertConsistent(SIConductance()); }
	};
	const SIConductance siemens = ampere/volt;	//< siemens

	/// Magnetic flux in SI system
	class SIMagFlux: public Unitful {
	public:
		/// constructor
		SIMagFlux(double t = 0): Unitful(t,volt*second) {}
		/// re-casting constructor
		SIMagFlux(const unit7_t& u): Unitful(1,u) { assertConsistent(SIMagFlux()); }
	};
	const SIMagFlux weber = volt*second;	//< weber

	/// Magnetic flux density
	class SIMagDensity: public Unitful {
	public:
		/// constructor
		SIMagDensity(double t = 0): Unitful(t,weber/(meter*meter)) {}
		/// re-casting constructor
		SIMagDensity(const unit7_t& u): Unitful(1,u) { assertConsistent(SIMagDensity()); }
	};
	const SIMagDensity tesla = weber/(meter*meter);	//< tesla
	const SIMagDensity gauss = tesla/10000;			//< gauss

	/// Inductance in SI system
	class SIInductance: public Unitful {
	public:
		/// constructor
		SIInductance(double t = 0): Unitful(t,weber/ampere) {}
		/// re-casting constructor
		SIInductance(const unit7_t& u): Unitful(1,u) { assertConsistent(SIInductance()); }
	};
	const SIInductance henry = weber/ampere;	//< henry


}

#endif
