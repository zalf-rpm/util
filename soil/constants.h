/**
Authors:
Claas Nendel <michael.berg@zalf.de>

Maintainers:
Currently maintained by the authors.

This file is part of the util library used by models created at the Institute of
Landscape Systems Analysis at the ZALF.
Copyright (C) 2007-2013, Leibniz Centre for Agricultural Landscape Research (ZALF)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOIL_CONSTANTS_H_
#define SOIL_CONSTANTS_H_

namespace Soil
{
  //! @author Claas Nendel
  struct OrganicConstants
  {
    static const double po_UreaMolecularWeight; //!< 0.06006; //!< [kg mol-1]
    static const double po_Urea_to_N; //!< 0.46667; ; //!< Converts 1 kg urea to 1 kg N
    static const double po_NH3MolecularWeight; //!< 0.01401; //!< [kg mol-1]
    static const double po_NH4MolecularWeight; //!< 0.01401; //!< [kg mol-1]
    static const double po_H2OIonConcentration; //!< 1.0;
    static const double po_pKaHNO2; //!< 3.29; //!< [] pKa value for nitrous acid
    static const double po_pKaNH3; //!< 6.5; //!< [] pKa value for ammonium
    static const double po_SOM_to_C; //!< 0.57; //!< 0.58; [] converts soil organic matter to carbon
    static const double po_AOM_to_C; //!< 0.45; //!< [] converts added organic matter to carbon
  };
}

#endif
