#pragma once

#include "yaml.h"
#include <iostream>
#include <string>

const std::string CONF_FILE = "../conf/config.yaml";

/* --------------- Declaration --------------- */
class Config {
public:
  static Config *getInstance();

private:
  explicit Config();

public:
  YAML::Node conf_msg;

private:
  static Config *instance_;
};

/* --------------- Definition --------------- */
Config *Config::instance_ = new (std::nothrow) Config();

Config *Config::getInstance() {
  // if (Config::instance_ == nullptr) {
  //   Config::instance_ = new (std::nothrow) Config();
  // }
  return Config::instance_;
}

Config::Config() { this->conf_msg = YAML::LoadFile(CONF_FILE); }
