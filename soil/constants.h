/**
Authors:
Michael Berg <michael.berg@zalf.de>

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

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

namespace Soil
{
  /**
   * @author Claas Nendel
   */
  struct OrganicConstants
  {
    static const double po_UreaMolecularWeight;
    static const double po_Urea_to_N;
    static const double po_NH3MolecularWeight;
    static const double po_NH4MolecularWeight;
    static const double po_H2OIonConcentration;
    static const double po_pKaHNO2;
    static const double po_pKaNH3;
    static const double po_SOM_to_C;
    static const double po_AOM_to_C;
  };
}

#endif
