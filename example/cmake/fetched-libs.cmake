set(FETCHCONTENT_QUIET FALSE)
include(FetchContent)

# glbinding
# ---------
FetchContent_Declare(
  glbinding
  ULR_HASH SHA256=b959b8961930258244a8e0061d33d8bcd24f974fd54e9e72689a871788c82ab6
  URL https://github.com/cginternals/glbinding/archive/refs/tags/v3.5.0.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP ON)
FetchContent_MakeAvailable(glbinding)

# glm
# ---
FetchContent_Declare(
  glm
  ULR_HASH SHA256=sha256:a41adf423315e35489c1e67689c7bb74c7edd4d207a477b7b19c45cd440c2f67
  URL https://github.com/g-truc/glm/releases/download/1.0.2/glm-1.0.2.zip
  DOWNLOAD_EXTRACT_TIMESTAMP ON)
FetchContent_MakeAvailable(glm)

# imgui
# -----
FetchContent_Declare(
  dear_imgui
  URL_HASH SHA256=0eb50fe9aeba1a51f96b5843c7f630a32ed2e9362d693c61b87e4fa870cf826d
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.92.5.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP ON)
FetchContent_MakeAvailable(dear_imgui)

add_library(
  dear_imgui
  STATIC
    ${dear_imgui_SOURCE_DIR}/imgui.cpp
    ${dear_imgui_SOURCE_DIR}/imgui_draw.cpp
    ${dear_imgui_SOURCE_DIR}/imgui_tables.cpp
    ${dear_imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${dear_imgui_SOURCE_DIR}/imgui_demo.cpp
    ${dear_imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${dear_imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(
  dear_imgui
  PUBLIC
    ${dear_imgui_SOURCE_DIR}/
    ${dear_imgui_SOURCE_DIR}/backends/)
target_link_libraries(dear_imgui PRIVATE $<COMPILE_ONLY:glfw>)

# linmath
# -------
FetchContent_Declare(
  linmath
  GIT_REPOSITORY https://github.com/datenwolf/linmath.h
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_MakeAvailable(linmath)

add_library(linmath INTERFACE)
target_include_directories(linmath INTERFACE ${linmath_SOURCE_DIR}/)

# stb
# ---
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb/
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_MakeAvailable(stb)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR}/)

# tinycthread
# -----------
FetchContent_Declare(
  tinycthread
  GIT_REPOSITORY https://github.com/tinycthread/tinycthread
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_MakeAvailable(tinycthread)

# getopt
# -----------
FetchContent_Declare(
  getopt
  GIT_REPOSITORY https://github.com/kimgr/getopt_port
  GIT_PROGRESS ON
  GIT_SHALLOW ON)
FetchContent_MakeAvailable(getopt)

add_library(getopt STATIC ${getopt_SOURCE_DIR}/getopt.c)
target_include_directories(getopt INTERFACE ${getopt_SOURCE_DIR}/)
