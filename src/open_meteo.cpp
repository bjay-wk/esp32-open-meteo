#include "open_meteo.hpp"
#include <algorithm>
#include <esp_crt_bundle.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <sstream>

#define TAG "OM_SDK"
#define PAST_DAY_MAX 92
#define FORCAST_DAY_MAX 16
#define WEB_URL "https://api.open-meteo.com"
#define FORECAST "/v1/forecast"
#define MAX_HTTP_OUTPUT_BUFFER_CALLOC 3 * 1024
#define ARRAY_LENGTH(array) (sizeof((array)) / sizeof((array)[0]))

namespace OM_SDK {

const char *const *EnumNamesTimeParams() {
  static const char *const names[] = {
      "undefined",
      "apparent_temperature",
      "apparent_temperature_max",
      "apparent_temperature_min",
      "cape",
      "cloud_cover",
      "cloud_cover_high",
      "cloud_cover_low",
      "cloud_cover_mid",
      "daylight_duration",
      "dew_point_2m",
      "diffuse_radiation",
      "direct_normal_irradiance",
      "direct_radiation",
      "et0_fao_evapotranspiration",
      "evapotranspiration",
      "freezing_level_height",
      "global_tilted_irradiance",
      "global_tilted_irradiance_instant",
      "is_day",
      "lightning_potential",
      "precipitation",
      "precipitation_hours",
      "precipitation_probability",
      "precipitation_probability_max",
      "precipitation_probability_mean",
      "precipitation_probability_min",
      "precipitation_sum",
      "pressure_msl",
      "rain",
      "rain_sum",
      "relative_humidity_2m",
      "shortwave_radiation",
      "shortwave_radiation_sum",
      "showers",
      "showers_sum",
      "snow_depth",
      "snowfall",
      "snowfall_height",
      "snowfall_sum",
      "soil_moisture_0_to_1cm",
      "soil_moisture_1_to_3cm",
      "soil_moisture_27_to_81cm",
      "soil_moisture_3_to_9cm",
      "soil_moisture_9_to_27cm",
      "soil_temperature_0cm",
      "soil_temperature_18cm",
      "soil_temperature_54cm",
      "soil_temperature_6cm",
      "sunrise",
      "sunset",
      "sunshine_duration",
      "surface_pressure",
      "temperature_2m",
      "temperature_2m_max",
      "temperature_2m_min",
      "uv_index_clear_sky_max",
      "uv_index_max",
      "vapour_pressure_deficit",
      "visibility",
      "weather_code",
      "wind_direction_10m",
      "wind_direction_10m_dominant",
      "wind_direction_120m ",
      "wind_direction_180m ",
      "wind_direction_80m ",
      "wind_gusts_10m ",
      "wind_gusts_10m_max ",
      "wind_speed_10m ",
      "wind_speed_10m_max ",
      "wind_speed_120m ",
      "wind_speed_180m",
      "wind_speed_80m",
      "uv_index",
      "max_params",
      nullptr,
  };
  return names;
}

const char *const *EnumNamesTemperatureUnit() {
  static const char *const names[] = {
      "undefined_tmp_unit",
      "celsius",
      "fahrenheit",
      nullptr,
  };
  return names;
}

const char *const *EnumNamesWindSpeedUnit() {
  static const char *const names[] = {
      "undefined_wind_unit", "kmh", "ms", "mph", "kn", nullptr,
  };
  return names;
}

const char *const *EnumNamesPrecipitationUnit() {
  static const char *const names[] = {
      "undefined_precipitation_unit",
      "mm",
      "inch",
      nullptr,
  };
  return names;
}

const char *const *EnumNamesTimeFormat() {
  static const char *const names[] = {
      "undefined_timeformat",
      "iso8601",
      "unixtime",
      nullptr,
  };
  return names;
}

const char *const *EnumNamesCellSelection() {
  static const char *const names[] = {
      "undefined_selection", "land", "sea", "nearest", nullptr,
  };
  return names;
}

const char *EnumNamesWeatherCode(WeatherCode code) {
  switch (code) {
  case Clear_sky:
    return "Clear sky";
  case mainly_clear:
    return "Mainly clear";
  case partly_cloudy:
    return "Partly_cloudy";
  case overcast:
    return "Overcast";
  case fog:
    return "Fog";
  case depositing_rime_fog:
    return "Depositing rime fog";
  case drizzle_light:
    return "Light drizzle";
  case drizzle_moderate:
    return "Moderate drizzle";
  case drizzle_dense:
    return "Dense drizzle";
  case freezing_drizzle_light:
    return "Light freezing drizzle";
  case Freezing_drizzle_dense:
    return "Dense freezing drizzle";
  case ain_slight:
    return "slight rain";
  case rain_moderate:
    return "Moderate rain";
  case rain_heavy_intensity:
    return "Heavy rain";
  case freezing_rain_light:
    return "Light freezing rain";
  case freezing_rain_heavy:
    return "Heavy freezing rain";
  case snow_fall_slight:
    return "slight snow fall";
  case snow_fall_moderate:
    return "Moderate snow fall";
  case snow_fall_heavy:
    return "Heavy snow fall";
  case snow_grains:
    return "snow grains";
  case rain_showers_Slight:
    return "Slight rain showers";
  case rain_showers_moderate:
    return "Moderate rain showers";
  case rain_showers_violent:
    return "Violent rain showers";
  case snow_showers_slight:
    return "slight snow showers";
  case snow_showersheavy:
    return "Heavy snow showers";
  case thunderstorm_slight_moderate:
    return "slight or moderate thunderstorm";
  case thunderstorm_slight_hail:
    return "slight hail thunderstorm";
  case thunderstorm_heavy_hail:
    return "Heavy hail thunderstorm";
  default:
    break;
  }
  ESP_LOGI(TAG, "unknown case :%i", code);
  return "unknown";
}

const TimeParam currentFilter[] = {
    undefined,
    apparent_temperature,
    cape,
    cloud_cover,
    cloud_cover_high,
    cloud_cover_low,
    cloud_cover_mid,
    dew_point_2m,
    diffuse_radiation,
    direct_normal_irradiance,
    direct_radiation,
    et0_fao_evapotranspiration,
    evapotranspiration,
    freezing_level_height,
    global_tilted_irradiance,
    global_tilted_irradiance_instant,
    is_day,
    lightning_potential,
    precipitation,
    precipitation_probability,
    pressure_msl,
    rain,
    relative_humidity_2m,
    shortwave_radiation,
    showers,
    snow_depth,
    snowfall,
    snowfall_height,
    soil_moisture_0_to_1cm,
    soil_moisture_1_to_3cm,
    soil_moisture_27_to_81cm,
    soil_moisture_3_to_9cm,
    soil_moisture_9_to_27cm,
    soil_temperature_0cm,
    soil_temperature_18cm,
    soil_temperature_54cm,
    soil_temperature_6cm,
    sunshine_duration,
    surface_pressure,
    temperature_2m,
    vapour_pressure_deficit,
    visibility,
    weather_code,
    wind_direction_10m,
    wind_direction_120m,
    wind_direction_180m,
    wind_direction_80m,
    wind_gusts_10m,
    wind_speed_10m,
    wind_speed_120m,
    wind_speed_180m,
    wind_speed_80m,
};

const TimeParam hourlyFilter[] = {
    undefined,
    temperature_2m,
    relative_humidity_2m,
    dew_point_2m,
    apparent_temperature,
    pressure_msl,
    surface_pressure,
    cloud_cover,
    cloud_cover_low,
    cloud_cover_mid,
    cloud_cover_high,
    wind_speed_10m,
    wind_speed_80m,
    wind_speed_120m,
    wind_speed_180m,
    wind_direction_10m,
    wind_direction_80m,
    wind_direction_120m,
    wind_direction_180m,
    wind_gusts_10m,
    shortwave_radiation,
    direct_radiation,
    direct_normal_irradiance,
    diffuse_radiation,
    global_tilted_irradiance,
    vapour_pressure_deficit,
    cape,
    evapotranspiration,
    et0_fao_evapotranspiration,
    precipitation,
    snowfall,
    precipitation_probability,
    rain,
    showers,
    weather_code,
    snow_depth,
    freezing_level_height,
    visibility,
    soil_temperature_0cm,
    soil_temperature_6cm,
    soil_temperature_18cm,
    soil_temperature_54cm,
    soil_moisture_0_to_1cm,
    soil_moisture_1_to_3cm,
    soil_moisture_3_to_9cm,
    soil_moisture_9_to_27cm,
    soil_moisture_27_to_81cm,
    is_day,
    uv_index,
};

const TimeParam minutely_15Filter[]{
    undefined,
    temperature_2m,
    relative_humidity_2m,
    dew_point_2m,
    apparent_temperature,
    shortwave_radiation,
    direct_radiation,
    direct_normal_irradiance,
    global_tilted_irradiance,
    global_tilted_irradiance_instant,
    diffuse_radiation,
    sunshine_duration,
    lightning_potential,
    precipitation,
    snowfall,
    rain,
    showers,
    snowfall_height,
    freezing_level_height,
    cape,
    wind_speed_10m,
    wind_speed_80m,
    wind_direction_10m,
    wind_direction_80m,
    wind_gusts_10m,
    visibility,
    weather_code,
};

const TimeParam dailyFilter[] = {
    undefined,
    temperature_2m_max,
    temperature_2m_min,
    apparent_temperature_max,
    apparent_temperature_min,
    precipitation_sum,
    rain_sum,
    showers_sum,
    snowfall_sum,
    precipitation_hours,
    precipitation_probability_max,
    precipitation_probability_min,
    precipitation_probability_mean,
    weather_code,
    sunrise,
    sunset,
    sunshine_duration,
    daylight_duration,
    wind_speed_10m_max,
    wind_gusts_10m_max,
    wind_direction_10m_dominant,
    shortwave_radiation_sum,
    et0_fao_evapotranspiration,
    uv_index_max,
    uv_index_clear_sky_max,
};

void filterTimeParams(TimeParam *params, const TimeParam *filter,
                      const int filter_size) {
  if (!params || !filter)
    return;
  TimeParam *value = params;
  while (value && *value != max_params) {
    int i = 0;
    while (i < filter_size && filter[i] != *value) {
      ++i;
    }
    if (i == filter_size) {
      *value = undefined;
    }
    ++value;
  }
}

void validate_time_interval(time_t *t1, time_t *t2) {
  if (*t1 == 0 && *t2 == 0)
    return;
  if (*t1 == 0)
    time(t1);
  if (*t2 == 0)
    time(t2);
  if (t1 > t2) {
    std::swap(t1, t2);
  }
}

void validateParams(OpenMeteoParams *params) {
  filterTimeParams(params->hourly, hourlyFilter, ARRAY_LENGTH(hourlyFilter));
  filterTimeParams(params->daily, dailyFilter, ARRAY_LENGTH(dailyFilter));
  filterTimeParams(params->minutely_15, minutely_15Filter,
                   ARRAY_LENGTH(minutely_15Filter));
  filterTimeParams(params->current, currentFilter, ARRAY_LENGTH(currentFilter));
  if (params->past_days >= PAST_DAY_MAX) {
    params->past_days = PAST_DAY_MAX;
  }
  if (params->forecast_days >= FORCAST_DAY_MAX) {
    params->forecast_days = FORCAST_DAY_MAX;
  }
  validate_time_interval(&params->start_date, &params->end_date);
  validate_time_interval(&params->start_hour, &params->end_hour);
  validate_time_interval(&params->start_minutely_15, &params->end_minutely_15);
}

std::string add(time_t value, const char *name, const char *format) {
  struct tm timeinfo;
  localtime_r(&value, &timeinfo);
  char strftime_buf[17] = {0};
  strftime(strftime_buf, ARRAY_LENGTH(strftime_buf), format, &timeinfo);
  std::stringstream ss;
  if (value != 0)
    ss << "&" << name << "=" << strftime_buf;
  return ss.str();
}

std::string add(int8_t value, const char *name) {
  std::stringstream ss;
  if (value > 0)
    ss << "&" << name << "=" << (int)value;
  return ss.str();
}

std::string timeParamstoString(TimeParam *params) {
  if (!params)
    return "";
  std::stringstream ss;
  TimeParam *param = params;
  while (param && *param != max_params) {
    if (*param != undefined) {
      ss << EnumNamesTimeParams()[*param];
      break;
    }
    param++;
  }
  param++;
  while (param && *param != max_params) {
    if (*param != undefined)
      ss << "," << EnumNamesTimeParams()[*param];
    param++;
  }
  return ss.str();
}

std::string timeParams_to_args(TimeParam *params, const char *str) {
  if (!params)
    return "";
  std::stringstream ss;
  std::string value = timeParamstoString(params);
  if (value.empty())
    return "";
  ss << str << value;
  return ss.str();
}

std::string paramsToString(const OpenMeteoParams *p) {
  std::stringstream ss;
  ss << "?latitude=" << p->latitude << "&longitude=" << p->longitude
     << "&format=flatbuffers";
  if (strcmp(CONFIG_OPEN_METEO_API_KEY, "")) {
    ss << ",apikey=" CONFIG_OPEN_METEO_API_KEY;
  }
  bool force_timezone_to_auto = false;
  if (!p->elevation_default)
    ss << "&elevation=" << p->elevation;
  ss << timeParams_to_args(p->hourly, "&hourly=")
     << timeParams_to_args(p->minutely_15, "&minutely_15=")
     << timeParams_to_args(p->current, "&current=");
  if (p->daily) {
    ss << timeParams_to_args(p->daily, "&daily=");
    force_timezone_to_auto = true;
  }
  if (p->temperature_unit != undefined_tmp_unit)
    ss << "&temperature_unit="
       << EnumNamesTemperatureUnit()[p->temperature_unit];
  if (p->wind_speed_unit != undefined_wind_unit)
    ss << "&wind_speed_unit=" << EnumNamesWindSpeedUnit()[p->wind_speed_unit];
  if (p->precipitation_unit != undefined_precipitation_unit)
    ss << "&precipitation_unit="
       << EnumNamesPrecipitationUnit()[p->precipitation_unit];
  if (p->timeformat != undefined_timeformat)
    ss << "&timeformat=" << EnumNamesTimeFormat()[p->timeformat];
  if (force_timezone_to_auto) {
    ss << "&timezone=auto";
  } else if (p->timezone) {
    ss << "&timezone=" << p->timezone;
  }

  ss << add(p->past_days, "past_days") << add(p->forecast_days, "forecast_days")
     << add(p->forecast_hours, "forecast_hours")
     << add(p->forecast_minutely_15, "forecast_minutely_15")
     << add(p->past_hours, "past_hours")
     << add(p->past_minutely_15, "pas_minutely_15")
     << add(p->start_date, "start_date", "%F")
     << add(p->end_date, "end_date", "%F")
     << add(p->start_hour, "start_hour", "%FT%T")
     << add(p->end_hour, "end_hour", "%FT%T")
     << add(p->start_minutely_15, "start_minutely_15", "%FT%T")
     << add(p->end_minutely_15, "end_minutely_15", "%FT%T");
  if (p->models) {
    openmeteo_sdk::Model *models = p->models;
    ss << "&model=";
    if (*models != openmeteo_sdk::Model_undefined) {
      ss << EnumNameModel(*models);
      models++;
    }
    while (*models != openmeteo_sdk::Model_undefined) {
      ss << "," << EnumNameModel(*models);
      models++;
    }
  }
  if (p->cell_selection != undefined_selection)
    ss << "&cell_selection=" << EnumNamesCellSelection()[p->cell_selection];
  return ss.str();
}

int https_with_hostname_params(const char *path, const OpenMeteoParams *params,
                               openmeteo_sdk::WeatherApiResponse **output);

int get_weather(OpenMeteoParams *params,
                openmeteo_sdk::WeatherApiResponse **output) {
  if (!params)
    return -1;
  validateParams(params);
  return https_with_hostname_params(FORECAST, params, output);
}

int https_with_hostname_params(const char *path, const OpenMeteoParams *params,
                               openmeteo_sdk::WeatherApiResponse **output) {
  esp_http_client_config_t config = {};
  memset(&config, 0, sizeof(config));
  char *output_buffer = NULL;
  const std::string url = std::string(WEB_URL) + path + paramsToString(params);
  ESP_LOGI(TAG, "%s", url.c_str());
  config.url = url.c_str();
  config.crt_bundle_attach = esp_crt_bundle_attach;
  config.transport_type = HTTP_TRANSPORT_OVER_SSL;
  esp_http_client_handle_t client = esp_http_client_init(&config);
  esp_http_client_set_method(client, HTTP_METHOD_GET);
  int data_len = 0;
  char *data = NULL;
  esp_err_t err = esp_http_client_open(client, data_len);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
  } else if (data != NULL &&
             esp_http_client_write(client, data, data_len) < 0) {
    ESP_LOGE(TAG, "Write failed");
  } else if (esp_http_client_fetch_headers(client) < 0) {
    ESP_LOGE(TAG, "HTTP client fetch headers failed");
  } else {
    output_buffer = (char *)calloc(MAX_HTTP_OUTPUT_BUFFER_CALLOC, 1);
    int total_read = 0;
    int read = 0;
    do {
      read = esp_http_client_read_response(client, output_buffer + total_read,
                                           MAX_HTTP_OUTPUT_BUFFER_CALLOC -
                                               total_read);
      total_read += read;
    } while (read > 0);
    if (output) {
      *output = (openmeteo_sdk::WeatherApiResponse *)
          openmeteo_sdk::GetSizePrefixedWeatherApiResponse(output_buffer);
    }
    free(output_buffer);
  }

  esp_http_client_close(client);
  const int status_code = esp_http_client_get_status_code(client);
  esp_http_client_cleanup(client);
  return status_code;
}
} // namespace OM_SDK