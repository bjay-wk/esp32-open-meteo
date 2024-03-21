# esp32-open-meteo

Open meteo Api for espidf.

[Weather data by Open-Meteo.com](https://open-meteo.com/)

## How to Add to you project
### Using platformio

- Add `https://github.com/bjay-wk/esp32-open-meteo.git` to the `lib_deps`in `platformio.ini`.
- In the CMakeLists.txt in your root you have to add before project(...):
  ```cmake
  get_filename_component(configName "${CMAKE_BINARY_DIR}" NAME)
  FILE(GLOB_RECURSE kconfigs_pio_lib_deps ${CMAKE_SOURCE_DIR}/.pio/libdeps/${configName}/*/Kconfig)
  list(APPEND kconfigs ${kconfigs_pio_lib_deps})
  ```

### Using Espidf
In `src/idf_component.yml` of your project add

```yaml
esp32-open-meteo:
  git: https://github.com/bjay-wk/esp32-open-meteo.git
```

And in `src/CMakeLists.txt` of your project

```make
idf_component_register(...
    REQUIRES esp32-open-meteo
    ...
)
```