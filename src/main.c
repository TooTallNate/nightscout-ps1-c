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

#define RED      L"\001\x1b[31m\002"
#define GREEN    L"\001\x1b[32m\002"
#define YELLOW   L"\001\x1b[33m\002"
#define BLUE     L"\001\x1b[34m\002"
#define MAGENTA  L"\001\x1b[35m\002"
#define CYAN     L"\001\x1b[36m\002"
#define NO_COLOR L"\001\x1b[0m\002"
#define BOLD     L"\001\x1b[1m\002"
#define NO_BOLD  L"\001\x1b[22m\002"

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
  /* latest entry */
  if (MATCH("latest_entry", "mgdl")) {
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
  wint_t buf[1024] = { 0 };
  char latest_entry_path[1024];

  struct passwd *pw = getpwuid(getuid());

  snprintf(latest_entry_path, sizeof(latest_entry_path), "%s/%s", pw->pw_dir, ".nightscout-latest-entry");

  if (ini_parse(latest_entry_path, handler, &s) < 0) {
    printf("Can't load '%s'\n", latest_entry_path);
    return 1;
  }

  gettimeofday(&now, NULL);
  long unsigned ms_ago = tv2ms(&now) - s.mills;

  const wint_t* color;
  int strike = 0;
  wint_t trend = L'?';
  wint_t padded_mgdl[4] = { 0 };
  swprintf(padded_mgdl, sizeof(padded_mgdl), L"%03d", s.mgdl);

  if (s.alarm_timeago_urgent && ms_ago > s.alarm_timeago_urgent_mins * MS_PER_MINUTE) {
    trend = L'↛';
    strike = 1;
    color = RED BOLD;
  } else if (s.alarm_timeago_warn && ms_ago > s.alarm_timeago_warn_mins * MS_PER_MINUTE) {
    trend = L'↛';
    strike = 1;
    color = YELLOW BOLD;
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
  i += swprintf(buf + i, sizeof(buf) - (sizeof(wint_t) * i), L"%S", color);
  if (strike) {
    i += strikethrough(padded_mgdl, buf + i, sizeof(buf) - (sizeof(wint_t) * i));
  } else {
    i += swprintf(buf + i, sizeof(buf) - (sizeof(wint_t) * i), L"%S", padded_mgdl);
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
