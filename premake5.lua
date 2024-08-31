local PROJECT = {
    SOURCE =    { "src/main.cpp" },
    LIB =     { "lib/subhook/subhook.c"--[[, "lib/subhook/subhook_x86.c", "lib/subhook/subhook.h"]] }
}

workspace "gm_as2"
  location "project"
  configurations { "Release" }

configuration "Release"
  optimize "On"

project "gm_as2"
  location "project"
  kind "SharedLib"
  flags "StaticRuntime"
  language "C++"
  architecture "x86"
  targetdir "bin/"
  cppdialect "C++17"
  characterset "ASCII"
   
  files { PROJECT.SOURCE, PROJECT.LIB }

  defines { "SUBHOOK_IMPLEMENTATION" }