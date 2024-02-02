vcpkg_check_linkage(ONLY_STATIC_LIBRARY) # this mirrors ImGui's portfile behavior

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO pthom/hello_imgui
    REF 40adf4bef4e0cbac4dfcbde1a40a79966a2ba5ec
    SHA512 28f3293483dae0b178f8d0648d6b5c58fec979bc297d3af1c780d2714fb3fbe5ca22a314e19b3bf43d933b57f4ac26b98f9c67a4e9da22599c1d95eebe5d05b5
    HEAD_REF master
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
    "opengl3-binding" FEATURE_OPENGL3_BINDING
    "metal-binding" FEATURE_METAL_BINDING
    "experimental-vulkan-binding" FEATURE_VULKAN_BINDING
    "experimental-dx11-binding" FEATURE_DX11_BINDING
    "experimental-dx12-binding" FEATURE_DX12_BINDING
    "glfw-binding" FEATURE_GLFW_BINDING
    "sdl2-binding" FEATURE_SDL2_BINDING
    "freetype-lunasvg" HELLOIMGUI_USE_FREETYPE # When hello_imgui is built with freetype, it will also build with lunasvg
)

# if a renderer backend was selected and is different from the default, we need to disable the default
if(FEATURE_METAL_BINDING AND FEATURE_OPENGL3_BINDING)
    message(STATUS "Metal and OpenGL3 bindings are mutually exclusive. Removing support for OpenGL3.")
    set(FEATURE_OPENGL3_BINDING OFF)
endif()
if(FEATURE_VULKAN_BINDING AND FEATURE_OPENGL3_BINDING)
    message(STATUS "Vulkan and OpenGL3 bindings are mutually exclusive. Removing support for OpenGL3.")
    set(FEATURE_OPENGL3_BINDING OFF)
endif()
if(FEATURE_DX11_BINDING AND FEATURE_OPENGL3_BINDING)
    message(STATUS "Dx11 and OpenGL3 bindings are mutually exclusive. Removing support for OpenGL3.")
    set(FEATURE_OPENGL3_BINDING OFF)
endif()
if(FEATURE_DX12_BINDING AND FEATURE_OPENGL3_BINDING)
    message(STATUS "Dx12 and OpenGL3 bindings are mutually exclusive. Removing support for OpenGL3.")
    set(FEATURE_OPENGL3_BINDING OFF)
endif()


# Set HelloImGui backend combinations (rendering + platform)
if(FEATURE_OPENGL3_BINDING AND FEATURE_GLFW_BINDING)
    set(HELLOIMGUI_USE_GLFW_OPENGL3 ON)
endif()
if(FEATURE_OPENGL3_BINDING AND FEATURE_SDL2_BINDING)
    set(HELLOIMGUI_USE_SDL_OPENGL3 ON)
endif()
if(FEATURE_METAL_BINDING AND FEATURE_SDL2_BINDING)
    set(HELLOIMGUI_USE_SDL_METAL ON)
endif()
if(FEATURE_METAL_BINDING AND FEATURE_GLFW_BINDING)
    set(HELLOIMGUI_USE_GLFW_METAL ON)
endif()
if(FEATURE_VULKAN_BINDING AND FEATURE_GLFW_BINDING)
    set(HELLOIMGUI_USE_GLFW_VULKAN ON)
endif()
if(FEATURE_VULKAN_BINDING AND FEATURE_SDL2_BINDING)
    set(HELLOIMGUI_USE_SDL_VULKAN ON)
endif()
if(FEATURE_DX11_BINDING AND FEATURE_SDL2_BINDING)
    set(HELLOIMGUI_USE_SDL_DIRECTX11 ON)
endif()
if(FEATURE_DX11_BINDING AND FEATURE_GLFW_BINDING)
    set(HELLOIMGUI_USE_GLFW_DIRECTX11 ON)
endif()
if(FEATURE_DX12_BINDING AND FEATURE_SDL2_BINDING)
    set(HELLOIMGUI_USE_SDL_DIRECTX12 ON)
endif()
if(FEATURE_DX12_BINDING AND FEATURE_GLFW_BINDING)
    set(HELLOIMGUI_USE_GLFW_DIRECTX12 ON)
endif()

set(platform_options "")
if(WIN32)
    # Standard win32 options (these are the defaults for HelloImGui)
    # we could add a vcpkg feature for this, but it would have to be platform specific
    list(APPEND platform_options
        -DHELLOIMGUI_WIN32_NO_CONSOLE=ON
        -DHELLOIMGUI_WIN32_AUTO_WINMAIN=ON
    )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Standard macOS options (these are the defaults for HelloImGui)
    # we could add a vcpkg feature for this, but it would have to be platform specific
    list(APPEND platform_options
        -DHELLOIMGUI_MACOS_NO_BUNDLE=OFF
    )
endif()


vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DHELLOIMGUI_BUILD_DEMOS=OFF
        -DHELLOIMGUI_BUILD_DOCS=OFF
        -DHELLOIMGUI_BUILD_TESTS=OFF

        # vcpkg does not support ImGui Test Engine, so we cannot enable it
        -DHELLOIMGUI_WITH_TEST_ENGINE=OFF

        -DHELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE=ON
        -DHELLO_IMGUI_IMGUI_SHARED=OFF
        -DHELLOIMGUI_BUILD_IMGUI=OFF

        ${platform_options}

        # Backend combinations (hello_imgui wants a combination of rendering and platform backend)
        # (we can select at most one rendering backend)
        -DHELLOIMGUI_USE_GLFW_OPENGL3=${HELLOIMGUI_USE_GLFW_OPENGL3}
        -DHELLOIMGUI_USE_SDL_OPENGL3=${HELLOIMGUI_USE_SDL_OPENGL3}
        -DHELLOIMGUI_USE_SDL_METAL=${HELLOIMGUI_USE_SDL_METAL}
        -DHELLOIMGUI_USE_GLFW_METAL=${HELLOIMGUI_USE_GLFW_METAL}
        -DHELLOIMGUI_USE_GLFW_VULKAN=${HELLOIMGUI_USE_GLFW_VULKAN}
        -DHELLOIMGUI_USE_SDL_VULKAN=${HELLOIMGUI_USE_SDL_VULKAN}
        -DHELLOIMGUI_USE_SDL_DIRECTX11=${HELLOIMGUI_USE_SDL_DIRECTX11}
        -DHELLOIMGUI_USE_GLFW_DIRECTX11=${HELLOIMGUI_USE_GLFW_DIRECTX11}
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/hello_imgui PACKAGE_NAME "hello-imgui")  # should be active once himgui produces a config

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share"
    "${CURRENT_PACKAGES_DIR}/share/hello-imgui/hello_imgui_cmake/ios-cmake"
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
