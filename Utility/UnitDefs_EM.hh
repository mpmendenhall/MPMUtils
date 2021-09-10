/// \file UnitDefs_EM.hh Electromagnetics units

#ifndef UNITDEFS_EM_HH
#define UNITDEFS_EM_HH

#include "UnitDefs_Base.hh"

namespace Units {

    const Unitful coulomb = ampere*second;      ///< coulomb (charge)

    const Unitful volt = joule/coulomb;         ///< volt (electrical potential)

    const Unitful farad = coulomb/volt;         ///< farad (capacitance)

    const Unitful ohm = volt/ampere;            ///< ohm (resistance)

    const Unitful siemens = ampere/volt;        ///< siemens (conductivity)

    const Unitful weber = volt*second;          ///< weber (magnetic flux)

    const Unitful tesla = weber/(meter*meter);  ///< tesla (magnetic density)
    const Unitful gauss(1e-4, tesla);           ///< gauss (magnetic density)

    const Unitful henry = weber/ampere;         ///< henry (inductance)
}

#endif
