//
// File: mvm_fw_unit_test_config.h
//
// Author: Francesco Prelz (Francesco.Prelz@mi.infn.it)
//
// Revision history:
// 24-Apr-2020 Initial version.
// 29-Apr-2020 Timeline of text commands added.
//
// Description:
// Moved here the methods to access the JSON configuration file,
// with a static instance of the config handle, as this seems so fashionable
// around this project.
//

#ifndef _MVM_FW_TEST_CONFIG_H
#define _MVM_FW_TEST_CONFIG_H

#include <string>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

#include <cerrno>
#include <cstring> // strerror()

#include "SystemStatus.h"

#include "quantity_timelines.hpp"
struct sim_i2c_devaddr;

class system_error
{
  public:
    // We do this in the C way to keep to C++98 standard.
    system_error(): m_errs(::strerror(errno)) {}
    ~system_error() {}
    const std::string &get_err() const { return m_errs; }

  private:
    std::string m_errs;
};

std::ostream& operator<< (std::ostream &os, const system_error &serr);

const std::string MVM_FM_confattr_LogFile("LogFile");
const std::string MVM_FM_confattr_SerialTTY("SerialTTY");
const std::string MVM_FM_confattr_SerialPollTimeout("serial_port_timeout");
const std::string MVM_FM_confattr_StartTick("start_tick");
const std::string MVM_FM_confattr_EndTick("end_tick");
const std::string MVM_FM_confattr_EndMs("end_ms");
const std::string MVM_FM_confattr_CmdTimeline("command_timeline");
const std::string MVM_FM_confattr_MsScaleFactor("ms_scale_factor");
const std::string MVM_FM_confattr_UsWaitPerTick("us_wait_per_tick");
const std::string MVM_FM_confattr_DebugLevel("debug_level");

typedef std::map<qtl_tick_t, std::string> mvm_fw_test_cmds_t;
extern mvm_fw_test_cmds_t FW_TEST_command_timeline;

class mvm_fw_unit_test_config
{
  public:

    typedef rapidjson::Document mvm_fw_test_config_t;

    mvm_fw_unit_test_config(): m_valid(false), m_time_started(false) {}
    mvm_fw_unit_test_config(const std::string &conf_file): m_valid(false),
                                                    m_time_started(false)
     {
      load_config(conf_file);
     }
    ~mvm_fw_unit_test_config() {}

    bool get_string(const std::string &name,
                          std::string &value)
     {
      if (!m_valid) return false;
      const char *cname=name.c_str();
      if (m_conf.HasMember(cname))
       {
        const rapidjson::Value& v(m_conf[cname]);
        if (!v.IsString()) return false;
        value = v.GetString();
        return true;
       }
      return false;
     }

    template<typename TNUM>
    bool get_number(const std::string &name, TNUM &value)
     {
      if (!m_valid) return false;
      const char *cname=name.c_str();
      if (m_conf.HasMember(cname))
       {
        const rapidjson::Value& v(m_conf[cname]);
        if (!v.IsNumber()) return false;
        value = v.Get<TNUM>();
        return true;
       }
      return false;
     }

    template<typename TNUM>
    bool get_num_array(const std::string &name, TNUM *value, int size)
     {
      if (!m_valid) return false;
      const char *cname=name.c_str();
      if (m_conf.HasMember(cname))
       {
        const rapidjson::Value& a(m_conf[cname]);
        if (!a.IsArray()) return false;
        for (rapidjson::SizeType i = 0; ((i < a.Size())&&(i < size)); i++)
         {
          const rapidjson::Value& v(a[i]);
          if (!(v.IsNumber()))
           {
            value[i] = 0;
            continue;
           }
          value[i] = v.Get<TNUM>();
         }
        return true;
       }
      return false;
     }

    bool get_ushort_array(const std::string &name, uint16_t *value, int size)
     {
      // It seems that  no template classes can be instantiated for
      // uint16_t's.
      if (!m_valid) return false;
      const char *cname=name.c_str();
      if (m_conf.HasMember(cname))
       {
        const rapidjson::Value& a(m_conf[cname]);
        if (!a.IsArray()) return false;
        for (rapidjson::SizeType i = 0; ((i < a.Size())&&(i < size)); i++)
         {
          const rapidjson::Value& v(a[i]);
          if (!(v.IsNumber()))
           {
            value[i] = 0;
            continue;
           }
          value[i] = static_cast<uint16_t>(v.GetInt());
         }
        return true;
       }
      return false;
     }

    bool get_bool(const std::string &name, bool &value)
     {
      if (!m_valid) return false;
      const char *cname=name.c_str();
      if (m_conf.HasMember(cname))
       {
        const rapidjson::Value& v(m_conf[cname]);
        if (!v.IsBool()) return false;
        value = v.GetBool();
        return true;
       }
      return false;
     }

    bool load_config(const std::string &conf_file);
    const mvm_fw_test_config_t &get_conf() const { return m_conf; }

    const std::string &get_error_string() const { return m_error_string; }

    int load_command_timeline(mvm_fw_test_cmds_t &ctl,
                              const std::string &name=MVM_FM_confattr_CmdTimeline);

    void start_time()
     {
      if (!get_number<double>(MVM_FM_confattr_MsScaleFactor,
                              m_time_scale))
       {
        m_time_scale = 1.;
       }
      ::clock_gettime(CLOCK_REALTIME, &m_start_time);
      m_time_started = true;
     }

    timespec get_current_rt()
     {
      timespec now, ret;
      ::clock_gettime(CLOCK_REALTIME, &now);
      if (!m_time_started) return now;
      ret.tv_sec = now.tv_sec - m_start_time.tv_sec;
      int nsec_d = now.tv_nsec - m_start_time.tv_nsec;
      if (nsec_d < 0)
       {
        --(ret.tv_sec);
        ret.tv_nsec = nsec_d + 1000000000;
       }
      else ret.tv_nsec = nsec_d;
      return ret;
     }

    double get_scale_factor() const { return m_time_scale; }

    qtl_ms_t get_scaled_ms()
     {
      timespec cr=get_current_rt();
      qtl_ms_t ret = (cr.tv_sec * 1000) + (cr.tv_nsec/1000000);
      ret *= m_time_scale;
      return ret;
     }

  private:

    std::string m_conf_file;
    std::string m_error_string;
    bool m_valid;
    mvm_fw_test_config_t m_conf;
    bool m_time_started;
    double m_time_scale;
    timespec m_start_time;
};

extern quantity_timelines<double> FW_TEST_qtl_double;
extern qtl_tick_t                 FW_TEST_tick;
extern qtl_ms_t                   FW_TEST_ms;

extern t_SystemStatus            *FW_TEST_peek_system_status;
extern mvm_fw_unit_test_config    FW_TEST_main_config;

class
mvm_fw_gpio_devs
{
  public:
    mvm_fw_gpio_devs()
     {
      uint32_t pv1_init; // RapidJSON doesn't handle 16-bit types.
      if (!FW_TEST_main_config.get_number<uint32_t>("PV1_init", pv1_init))
       {
        m_pv1_value = 0; // Default: start w/ valve open.
       }
      else m_pv1_value = static_cast<uint16_t>(pv1_init);

      uint32_t gpio_init; // Default: valves open, no LED. 
      if (!FW_TEST_main_config.get_number<uint32_t>("GPIO_init", gpio_init))
       {
        gpio_init = 0; // Default: valves open, no LEDs. 
       }
      for (int i = 0; i<LAST_REG; ++i)
       {
        if (gpio_init & (1<<i)) m_devs[i] = true;
        else                    m_devs[i] = false;
       }
     }
    ~mvm_fw_gpio_devs() {}

    enum
    mvm_fw_bool_regs
     {
      BREATHE,
      OUT_VALVE,
      BUZZER,
      ALARM_LED,
      ALARM_RELAY,
      LAST_REG
     };

    bool set(mvm_fw_bool_regs dev, bool value)
     {
      double enable;
      bool ret = false;
      enable = FW_TEST_qtl_double.value(std::string(m_names[dev])+"_enable",
                                        FW_TEST_ms);
      timespec now;
      ::clock_gettime(CLOCK_REALTIME, &now);
      bool old_value = m_devs[dev];
      m_msg.str("");
      m_msg.clear();
      m_msg << "GPIO - DEVS" << " - " << m_names[dev] << " - "
            << now.tv_sec << ":" << now.tv_nsec/1000000
            << " - ms (scaled):" << FW_TEST_ms
            << " - tick:" << FW_TEST_tick << " - Alarms: "
            << std::hex << std::showbase 
            << FW_TEST_peek_system_status->ALARM_FLAG << " - Warnings: "
            << FW_TEST_peek_system_status->WARNING_FLAG << " - "
            << std::dec << std::noshowbase ;

      if (std::isnan(enable) ||
          (!std::isnan(enable) && (enable != 0)))
       {
        m_devs[dev] = value;
        m_msg << "value set to " << value;
        ret = true;
        if (m_devs[dev] == old_value)
         {
          // Don't log too verbosely.
          m_msg.str("");
          m_msg.clear();
         }
       }
      else
       {
        m_msg << "device disabled in config. NOT set to " << value
              << " - current value is " << m_devs[dev];
       }
      if (m_msg.str().length() > 0) m_msg << std::endl;
      return ret;
     }

    bool set_pv1(uint16_t value)
     {
      double enable;
      bool ret = false;
      enable = FW_TEST_qtl_double.value("PV1_enable", FW_TEST_ms);
      timespec now;
      ::clock_gettime(CLOCK_REALTIME, &now);
      uint16_t old_pv1_value = m_pv1_value;
      m_msg.str("");
      m_msg.clear();
      m_msg << "GPIO - DEVS - PV1 - "
      << now.tv_sec << ":" << now.tv_nsec/1000000
      << " - ms (scaled):" << FW_TEST_ms
      << " - tick:" << FW_TEST_tick << " - ";

      if (std::isnan(enable) ||
          (!std::isnan(enable) && (enable != 0)))
       {
        m_pv1_value = value;
        m_msg << "value set to " << value;
        ret = true;
        if (m_pv1_value == old_pv1_value)
         {
          // Don't log too verbosely.
          m_msg.str("");
          m_msg.clear();
         }
       }
      else
       {
        m_msg << "device disabled in config. NOT set to " << value
              << " - current value is " << m_pv1_value;
       }
      if (m_msg.str().length() > 0) m_msg << std::endl;
      return ret;
     }

    const std::string &get_error_msg()
     {
      m_msg_str = m_msg.str();
      return m_msg_str;
     }

    bool operator[](mvm_fw_bool_regs dev) const { return m_devs[dev]; }

    uint16_t get_pv1() const { return m_pv1_value; }
    double get_pv1_fraction() const
     {
      // Fraction of *opening* of the valve. Zero is valve *closed*
      // (confirmed on 20200506) - which is also the initial valve status.
      return ((static_cast<double>(m_pv1_value) / 0xffff));
     }

  private:
    bool m_devs[LAST_REG];
    std::ostringstream m_msg;
    std::string m_msg_str;
    uint16_t m_pv1_value;
    const char *m_names[LAST_REG] = { "BREATHE", "OUT_VALVE", "BUZZER",
                                      "ALARM_LED", "ALARM_RELAY" };
};

extern mvm_fw_gpio_devs FW_TEST_gdevs;

class
mvm_fw_unit_test_pflow
{
  public:
    mvm_fw_unit_test_pflow(): m_inited(false) {} 
    ~mvm_fw_unit_test_pflow() {}

    void   init();
    double p_value(const std::string &name, qtl_ms_t t);
    double in_f_value(qtl_ms_t t);
    double v_f_value(qtl_ms_t t);

    enum p_sensors
     {
      PS0,
      PS1,
      PS2,
      PS3,
      LAST_PS
     };

  private:
    bool m_inited;
    qtl_ms_t m_last_ms;
    void m_evolve(qtl_tick_t t);
    double m_m_resistance, m_in_v_resistance, m_out_v_resistance;
    double m_overpressure, m_peep;
    double m_capacity;
    double m_gas, m_gas_old;
    double m_in_flow, m_v_flow;
    double m_cur_p, m_old_c;
    double m_venturi_flow_at_1_psi_drop;
    double m_p[LAST_PS];
};

extern mvm_fw_unit_test_pflow FW_TEST_pflow;

/* Hardware map */

enum FW_TEST_devices
{
  TEST_TE_MS5525DSO,
  TEST_SENSIRION_SFM3019,
  TEST_TI_ADS1115,
  TEST_TCA_I2C_MULTIPLEXER,
  TEST_XXX_SUPERVISOR
};

typedef std::map<sim_i2c_devaddr, std::pair<FW_TEST_devices, std::string> > test_hardware_t;
extern test_hardware_t FW_TEST_hardware;
extern qtl_tick_t FW_TEST_last_watchdog_reset;

extern int FW_TEST_debug_level;
extern int FW_TEST_serial_poll_timeout;

#endif /* defined _MVM_FW_TEST_CONFIG_H */
