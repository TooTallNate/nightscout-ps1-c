#include <pwd.h>
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#include "uv.h"
#include "ini.h"

#define NO_COLOR L"\001\x1b[0m\002"
#define BOLD     L"\001\x1b[1m\002"
#define INVERSE  L"\001\x1b[7m\002"
#define RED      L"\001\x1b[31m\002"
#define RED      L"\001\x1b[31m\002"
#define GREEN    L"\001\x1b[32m\002"
#define YELLOW   L"\001\x1b[33m\002"
#define BLUE     L"\001\x1b[34m\002"
#define MAGENTA  L"\001\x1b[35m\002"
#define CYAN     L"\001\x1b[36m\002"

#define STRIKE 0x0336
#define MS_PER_MINUTE (1000 * 60)

/**
 * Convert struct timeval to milliseconds.
 *
 * \param tv The time value value to convert.
 *
 * \return The number off milliseconds in \a tv.
 */
long unsigned tv2ms(const struct timeval *tv) {
  return tv->tv_sec * 1000 + (tv->tv_usec + 500)/ 1000;
}

/* https://en.wikipedia.org/wiki/Strikethrough#Unicode */
int strikethrough(const wint_t* str, wint_t* buf, size_t size) {
  int written = 0;
  int len = wcslen(str);
  for (int i = 0; i < len; i++) {
    buf[written + 0] = str[i];
    buf[written + 1] = STRIKE;
    written += 2;
  }
  return written;
}

typedef struct
{
  /* previous entry */
  int previous_mgdl;
  long unsigned previous_mills;

  /* latest entry */
  int mgdl;
  long unsigned mills;
  const char* direction;

  /* stale entry alarms */
  int alarm_timeago_warn;
  int alarm_timeago_warn_mins;
  int alarm_timeago_urgent;
  int alarm_timeago_urgent_mins;

  /* thresholds */
  int bg_high;
  int bg_target_top;
  int bg_target_bottom;
  int bg_low;
} status;

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
  status* pStatus = (status*)user;

  #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
  #define PARSE_BOOL strcmp(value, "true") == 0
  /* previous entry */
  if (MATCH("previous_entry", "mgdl")) {
      pStatus->previous_mgdl = atoi(value);
  } else if (MATCH("previous_entry", "mills")) {
      pStatus->previous_mills = atoll(value);

  /* latest entry */
  } else if (MATCH("latest_entry", "mgdl")) {
      pStatus->mgdl = atoi(value);
  } else if (MATCH("latest_entry", "mills")) {
      pStatus->mills = atoll(value);
  } else if (MATCH("latest_entry", "direction")) {
      pStatus->direction = strdup(value);

  /* stale entry alarms */
  } else if (MATCH("settings", "alarm_timeago_warn")) {
      pStatus->alarm_timeago_warn = PARSE_BOOL;
  } else if (MATCH("settings", "alarm_timeago_warn_mins")) {
      pStatus->alarm_timeago_warn_mins = atoi(value);
  } else if (MATCH("settings", "alarm_timeago_urgent")) {
      pStatus->alarm_timeago_urgent = PARSE_BOOL;
  } else if (MATCH("settings", "alarm_timeago_urgent_mins")) {
      pStatus->alarm_timeago_urgent_mins = atoi(value);

  /* thresholds */
  } else if (MATCH("settings.thresholds", "bg_high")) {
      pStatus->bg_high = atoi(value);
  } else if (MATCH("settings.thresholds", "bg_target_top")) {
      pStatus->bg_target_top = atoi(value);
  } else if (MATCH("settings.thresholds", "bg_target_bottom")) {
      pStatus->bg_target_bottom = atoi(value);
  } else if (MATCH("settings.thresholds", "bg_low")) {
      pStatus->bg_low = atoi(value);

  } else {
      return 0;  /* unknown section/name, error */
  }

  return 1;
}

uv_loop_t *loop;
uv_tty_t tty;
int main(int argc, char* argv[]) {
  status s;
  struct timeval now;
  char *locale = setlocale(LC_ALL, "");

  loop = uv_default_loop();

  uv_tty_init(loop, &tty, 1, 0);
  uv_tty_set_mode(&tty, UV_TTY_MODE_NORMAL);

  int i = 0;
  wint_t buf[100] = { 0 };

  char homedir[1024];
  size_t size = sizeof(homedir);
  if (uv_os_homedir(homedir, &size)) {
    printf("Failed to read homedir\n");
    return 1;
  }

  char latest_entry_path[1024];
  snprintf(latest_entry_path, sizeof(latest_entry_path), "%s/%s", homedir, ".nightscout-latest-entry");

  if (ini_parse(latest_entry_path, handler, &s) < 0) {
    printf("Can't load '%s'\n", latest_entry_path);
    return 1;
  }

  gettimeofday(&now, NULL);
  long unsigned ms_ago = tv2ms(&now) - s.mills;

  const wint_t* color;
  wint_t trend = L'?';
  int strike = 0;
  int delta = s.mgdl - s.previous_mgdl;

  /* If the previous reading was more than 6 minutes ago (5 minutes is "normal",
     plus or minus some time to allow the reading to be uploaded */
  long delta_is_stale = s.mills - s.previous_mills > (MS_PER_MINUTE * 6);

  /* The mg/dl and delta from previous reading are put in their own buffer
     initially because they may be re-written with strikethrough in the end */
  wint_t mgdl_and_delta[12] = { 0 };
  i += swprintf(mgdl_and_delta, sizeof(mgdl_and_delta), L"%d %+d", s.mgdl, delta);
  if (delta_is_stale) {
    mgdl_and_delta[i++] = L'*';
  }

  /* Now calculate the color, trend, and whether or not to strikeout the values */
  if (s.alarm_timeago_urgent && ms_ago > s.alarm_timeago_urgent_mins * MS_PER_MINUTE) {
    trend = L'↛';
    strike = 1;
    color = INVERSE RED BOLD;
  } else if (s.alarm_timeago_warn && ms_ago > s.alarm_timeago_warn_mins * MS_PER_MINUTE) {
    trend = L'↛';
    strike = 1;
    color = INVERSE YELLOW BOLD;
  } else {
    if (strcmp(s.direction, "DoubleUp") == 0) {
      trend = L'⇈';
    } else if (strcmp(s.direction, "SingleUp") == 0) {
      trend = L'↑';
    } else if (strcmp(s.direction, "FortyFiveUp") == 0) {
      trend = L'↗';
    } else if (strcmp(s.direction, "Flat") == 0) {
      trend = L'→';
    } else if (strcmp(s.direction, "FortyFiveDown") == 0) {
      trend = L'↘';
    } else if (strcmp(s.direction, "SingleDown") == 0) {
      trend = L'↓';
    } else if (strcmp(s.direction, "DoubleDown") == 0) {
      trend = L'⇊';
    }

    if (s.mgdl > s.bg_high) {
      color = YELLOW BOLD;
    } else if (s.mgdl > s.bg_target_top) {
      color = YELLOW;
    } else if (s.mgdl < s.bg_low) {
      color = RED BOLD;
    } else if (s.mgdl < s.bg_target_bottom) {
      color = RED;
    } else {
      color = GREEN;
    }
  }

  /* fill buffer as wint_t */
  i = 0;
  i += swprintf(buf + i, sizeof(buf) - (sizeof(wint_t) * i), L"%S", color);
  if (strike) {
    i += strikethrough(mgdl_and_delta, buf + i, sizeof(buf) - (sizeof(wint_t) * i));
  } else {
    i += swprintf(buf + i, sizeof(buf) - (sizeof(wint_t) * i), L"%S", mgdl_and_delta);
  }
  i += swprintf(buf + i, sizeof(buf) - (sizeof(wint_t) * i), L" %C%S", trend, NO_COLOR);

  /* print buffer to stdout as UTF-8 through the libuv TTY machinery
     so that we get Windows normalization as well */
  char output[1024] = { 0 };
  snprintf(output, sizeof(output), "%S\n", buf);

  uv_write_t req;
  uv_buf_t buff;
  buff.base = output;
  buff.len = strlen(output);
  uv_write(&req, (uv_stream_t*) &tty, &buff, 1, NULL);
  uv_tty_reset_mode();

  return uv_run(loop, UV_RUN_DEFAULT);
}
