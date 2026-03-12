vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO SentoMK/patternia
  REF "v${VERSION}"
  SHA512 0
  HEAD_REF main
)

vcpkg_cmake_configure(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DPTN_BUILD_TESTS=OFF
    -DPTN_BUILD_BENCHMARKS=OFF
    -DPTN_BUILD_SAMPLES=OFF
    -DPTN_DEV_INDEX=OFF
    -DPTN_INSTALL=ON
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME patternia)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
