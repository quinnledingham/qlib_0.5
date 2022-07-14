//
// 4coder
//

// Project File
version(2);
project_name = "Q:/build/";

patterns = {
"*.c",
"*.cpp",
"*.h",
"*.m",
"*.bat",
"*.sh",
"*.4coder",
"*.menu",
};

blacklist_patterns = {
".*",
};

load_paths_base = {
 { ".", .relative = true, .recursive = true, },
};

load_paths = {
 .win = load_paths_base,
 .linux = load_paths_base,
 .mac = load_paths_base,
};

commands = {
 .build = { .out = "*compilation*", .footer_panel = true, .save_dirty_files = true,
   .win = "build.bat",
   .linux = "./build.sh",
   .mac = "./build.sh",}, 
};

fkey_command = {
.F1 = "run";
.F2 = "run";
};
// End of Project File

// End of 4coder

//
// Windows VS
//

@echo off

set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -wd4505 -wd4996 -wd4312 -wd4661 -DQLIB_INTERNAL=0 -DQLIB_SLOW=0 -DSAVE_IMAGES=1 -DQLIB_WIN32=1 -DVSYNC=0 -FC -Z7

set CommonLinkerFlags= -incremental:no -opt:ref user32.lib opengl32.lib gdi32.lib winmm.lib Dxgi.lib

pushd %CD%
IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

cl %CommonCompilerFlags% -DQLIB_OPENGL=1 ..\game\code\application.cpp -Fmwin32_application.map /link %CommonLinkerFlags%

popd

//
// Emscripten
//

// Start local server
python3 -m http.server