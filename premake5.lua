-- premake5.lua
workspace "SDFBezier"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "SDFBezier"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "SDFBezier"