# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#Authors: 
#Michael Berg <michael.berg@zalf.de>
#
#Maintainers: 
#Currently maintained by the authors.
#
#This file is part of the util library used by models created at the Institute of
#Landscape Systems Analysis at the ZALF.
#Copyright (C: Leibniz Centre for Agricultural Landscape Research (ZALF:

def humus_st2corg(humus_st):
	return \
  	{
			0: 0.0
		, 1: 0.5 / 1.72
		, 2: 1.5 / 1.72
		, 3: 3.0 / 1.72
		, 4: 6.0 / 1.72
		, 5: 11.5 / 2.0
		, 6: 17.5 / 2.0
		, 7: 30.0 / 2.0
		}.get(humus_st, 0.0)

def ld_eff2trd(ldEff, clay):
	x = \
		{
			1: 1.3
		, 2: 1.5
		, 3: 1.7
		, 4: 1.9
		, 5: 2.1
		}.get(ldEff, 0.0)

	return (x - (0.9 * clay)) * 1000.0 # *1000 = conversion from g cm-3 -> kg m-3

def sandAndClay2lambda(sand, clay):
	return (2.0 * (sand * sand * 0.575)) + (clay * 0.1) + ((1.0 - sand - clay) * 0.35)
	#return 1.0 #@todo <b>Claas:</b> Temporary override until we have solved the problem with low water percolation loam soils

def sandAndClay2KA5texture(sand, clay):
	silt = 1.0 - sand - clay
	soilTexture = ""

	if silt < 0.1 and clay < 0.05:
		soilTexture = "Ss"
	elif silt < 0.25 and clay < 0.05:
		soilTexture = "Su2"
	elif silt < 0.25 and clay < 0.08:
		soilTexture = "Sl2"
	elif silt < 0.40 and clay < 0.08:
		soilTexture = "Su3"
	elif silt < 0.50 and clay < 0.08:
		soilTexture = "Su4"
	elif silt < 0.8 and clay < 0.08:
		soilTexture = "Us"
	elif silt >= 0.8 and clay < 0.08:
		soilTexture = "Uu"
	elif silt < 0.1 and clay < 0.17:
		soilTexture = "St2"
	elif silt < 0.4 and clay < 0.12:
		soilTexture = "Sl3"
	elif silt < 0.4 and clay < 0.17:
		soilTexture = "Sl4"
	elif silt < 0.5 and clay < 0.17:
		soilTexture = "Slu"
	elif silt < 0.65 and clay < 0.17:
		soilTexture = "Uls"
	elif silt >= 0.65 and clay < 0.12:
		soilTexture = "Ut2"
	elif silt >= 0.65 and clay < 0.17:
		soilTexture = "Ut3"
	elif silt < 0.15 and clay < 0.25:
		soilTexture = "St3"
	elif silt < 0.30 and clay < 0.25:
		soilTexture = "Ls4"
	elif silt < 0.40 and clay < 0.25:
		soilTexture = "Ls3"
	elif silt < 0.50 and clay < 0.25:
		soilTexture = "Ls2"
	elif silt < 0.65 and clay < 0.30:
		soilTexture = "Lu"
	elif silt >= 0.65 and clay < 0.25:
		soilTexture = "Ut4"
	elif silt < 0.15 and clay < 0.35:
		soilTexture = "Ts4"
	elif silt < 0.30 and clay < 0.45:
		soilTexture = "Lts"
	elif silt < 0.50 and clay < 0.35:
		soilTexture = "Lt2"
	elif silt < 0.65 and clay < 0.45:
		soilTexture = "Tu3"
	elif silt >= 0.65 and clay >= 0.25:
		soilTexture = "Tu4"
	elif silt < 0.15 and clay < 0.45:
		soilTexture = "Ts3"
	elif silt < 0.50 and clay < 0.45:
		soilTexture = "Lt3"
	elif silt < 0.15 and clay < 0.65:
		soilTexture = "Ts2"
	elif silt < 0.30 and clay < 0.65:
		soilTexture = "Tl"
	elif silt >= 0.30 and clay < 0.65:
		soilTexture = "Tu2"
	elif clay >= 0.65:
		soilTexture = "Tt"
	else:
		soilTexture = ""

	return soilTexture

def KA5texture2sand(soilType):
	x = 0.0

	if soilType == "fS":
		x = 0.84
	elif soilType == "fSms":
		x = 0.86
	elif soilType == "fSgs":
		x = 0.88
	elif soilType == "gS":
		x = 0.93
	elif soilType == "mSgs":
		x = 0.96
	elif soilType == "mSfs":
		x = 0.93
	elif soilType == "mS":
		x = 0.96
	elif soilType == "Ss":
		x = 0.93
	elif soilType == "Sl2":
		x = 0.76
	elif soilType == "Sl3":
		x = 0.65
	elif soilType == "Sl4":
		x = 0.60
	elif soilType == "Slu":
		x = 0.43
	elif soilType == "St2":
		x = 0.84
	elif soilType == "St3":
		x = 0.71
	elif soilType == "Su2":
		x = 0.80
	elif soilType == "Su3":
		x = 0.63
	elif soilType == "Su4":
		x = 0.56
	elif soilType == "Ls2":
		x = 0.34
	elif soilType == "Ls3":
		x = 0.44
	elif soilType == "Ls4":
		x = 0.56
	elif soilType == "Lt2":
		x = 0.30
	elif soilType == "Lt3":
		x = 0.20
	elif soilType == "LtS":
		x = 0.42
	elif soilType == "Lu":
		x = 0.19
	elif soilType == "Uu":
		x = 0.10
	elif soilType == "Uls":
		x = 0.30
	elif soilType == "Us":
		x = 0.31
	elif soilType == "Ut2":
		x = 0.13
	elif soilType == "Ut3":
		x = 0.11
	elif soilType == "Ut4":
		x = 0.09
	elif soilType == "Utl":
		x = 0.19
	elif soilType == "Tt":
		x = 0.17
	elif soilType == "Tl":
		x = 0.17
	elif soilType == "Tu2":
		x = 0.12
	elif soilType == "Tu3":
		x = 0.10
	elif soilType == "Ts3":
		x = 0.52
	elif soilType == "Ts2":
		x = 0.37
	elif soilType == "Ts4":
		x = 0.62
	elif soilType == "Tu4":
		x = 0.05
	elif soilType == "L":
		x = 0.35
	elif soilType == "S":
		x = 0.93
	elif soilType == "U":
		x = 0.10
	elif soilType == "T":
		x = 0.17
	elif soilType == "HZ1":
		x = 0.30
	elif soilType == "HZ2":
		x = 0.30
	elif soilType == "HZ3":
		x = 0.30
	elif soilType == "Hh":
		x = 0.15
	elif soilType == "Hn":
		x = 0.15
	else:
		x = 0.66

	return x

def KA5texture2clay(soilType):
	x = 0.0

	if soilType == "fS":
		x = 0.02
	elif soilType == "fSms":
		x = 0.02
	elif soilType == "fSgs":
		x = 0.02
	elif soilType == "gS":
		x = 0.02
	elif soilType == "mSgs":
		x = 0.02
	elif soilType == "mSfs":
		x = 0.02
	elif soilType == "mS":
		x = 0.02
	elif soilType == "Ss":
		x = 0.02
	elif soilType == "Sl2":
		x = 0.06
	elif soilType == "Sl3":
		x = 0.10
	elif soilType == "Sl4":
		x = 0.14
	elif soilType == "Slu":
		x = 0.12
	elif soilType == "St2":
		x = 0.11
	elif soilType == "St3":
		x = 0.21
	elif soilType == "Su2":
		x = 0.02
	elif soilType == "Su3":
		x = 0.04
	elif soilType == "Su4":
		x = 0.04
	elif soilType == "Ls2":
		x = 0.21
	elif soilType == "Ls3":
		x = 0.21
	elif soilType == "Ls4":
		x = 0.21
	elif soilType == "Lt2":
		x = 0.30
	elif soilType == "Lt3":
		x = 0.40
	elif soilType == "Lts":
		x = 0.35
	elif soilType == "Lu":
		x = 0.23
	elif soilType == "Uu":
		x = 0.04
	elif soilType == "Uls":
		x = 0.12
	elif soilType == "Us":
		x = 0.04
	elif soilType == "Ut2":
		x = 0.10
	elif soilType == "Ut3":
		x = 0.14
	elif soilType == "Ut4":
		x = 0.21
	elif soilType == "Utl":
		x = 0.23
	elif soilType == "Tt":
		x = 0.82
	elif soilType == "Tl":
		x = 0.55
	elif soilType == "Tu2":
		x = 0.55
	elif soilType == "Tu3":
		x = 0.37
	elif soilType == "Ts3":
		x = 0.40
	elif soilType == "Ts2":
		x = 0.55
	elif soilType == "Ts4":
		x = 0.30
	elif soilType == "Tu4":
		x = 0.30
	elif soilType == "L":
		x = 0.31
	elif soilType == "S":
		x = 0.02
	elif soilType == "U":
		x = 0.04
	elif soilType == "T":
		x = 0.82
	elif soilType == "HZ1":
		x = 0.15
	elif soilType == "HZ2":
		x = 0.15
	elif soilType == "HZ3":
		x = 0.15
	elif soilType == "Hh":
		x = 0.1
	elif soilType == "Hn":
		x = 0.1

	return x

