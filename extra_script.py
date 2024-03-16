from os.path import isfile
import shutil
import os
Import("env")

flatbuffer_dir = "extra_lib/flatbuffers"
flatbuffer_build_dir = "build_flatbuffers"
weather_api = "include/weather_api_generated.h"

def remove_file(file_name):
  if isfile(file_name):
    os.remove(file_name)
    print(f"Removing {file_name}")
  elif os.path.isdir(file_name):
    shutil.rmtree(file_name)
    print(f"Removing {file_name}")

if env.IsCleanTarget():
  files = [flatbuffer_build_dir, weather_api]
  [remove_file(i) for i in files]

else:
  if not isfile(f"{flatbuffer_build_dir}/flatc"):
    env.Execute(f"cmake -G \"Unix Makefiles\" -DCMAKE_BUILD_TYPE=Release -S {flatbuffer_dir} -B {flatbuffer_build_dir}")
    env.Execute(f"make flatc -C {flatbuffer_build_dir} -j")
  if not isfile(weather_api):
    env.Execute(f"{flatbuffer_build_dir}/flatc -o include --cpp extra_lib/open-meteo-sdk/flatbuffers/weather_api.fbs")