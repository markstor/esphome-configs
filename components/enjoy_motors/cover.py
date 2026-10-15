import esphome.config_validation as cv

CONFIG_SCHEMA = cv.invalid(
    "The `cover: - platform: enjoy` platform is no longer supported."
    " Configure an `enjoy_motors:` command component and use its actions with"
    " `cover: - platform: time_based`."
)
