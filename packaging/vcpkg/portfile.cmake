vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO SentoMK/patternia
  REF "v${VERSION}"
  SHA512 f970ed1d6a4e5ce52c6e3de16d484fe999771696973495c02c38b1d2f35f7856ff7a6ac61e30eca16cf3b6ad1e3a2c26cb1079dd77451fc08464fc807546083f
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
