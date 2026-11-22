#include "../source/boat_radio_safety.h"
#include <assert.h>

int main() {
  BoatCommandDeduplicator d;
  assert(!d.hasExecuted(1,4,22));
  d.markExecuted(1,4,22);
  assert(d.hasExecuted(1,4,22));
  d.markExecuted(1,4,22); // repeated mark is idempotent
  assert(d.hasExecuted(1,4,22));
  assert(!d.hasExecuted(1,5,22)); // epoch scopes ID
  assert(!d.hasExecuted(2,4,22)); // sender scopes ID
  for(uint16_t id=100;id<132;id++) d.markExecuted(1,4,id);
  assert(!d.hasExecuted(1,4,22)); // bounded cache eviction is explicit
  assert(d.hasExecuted(1,4,131));
  d.clear();
  assert(!d.hasExecuted(1,4,131));

  assert(boatClassifyCrossRadioCommand(true,false,false)==BOAT_CROSSRADIO_NEW_COMMAND);
  assert(boatClassifyCrossRadioCommand(false,true,false)==BOAT_CROSSRADIO_DUPLICATE_EXECUTED);
  assert(boatClassifyCrossRadioCommand(false,false,true)==BOAT_CROSSRADIO_DUPLICATE_PENDING);
  assert(boatClassifyCrossRadioCommand(false,false,false)==BOAT_CROSSRADIO_DROP_STALE);

  BoatRadioWatchdog watchdog;
  assert(!watchdog.alive());
  assert(watchdog.update(false,false)==BOAT_RADIO_WATCHDOG_NO_CHANGE);
  assert(watchdog.update(true,false)==BOAT_RADIO_WATCHDOG_RESTORED);
  assert(watchdog.alive());
  assert(watchdog.update(false,true)==BOAT_RADIO_WATCHDOG_NO_CHANGE);
  assert(watchdog.update(false,false)==BOAT_RADIO_WATCHDOG_LOST);
  assert(!watchdog.alive());
  assert(watchdog.update(false,false)==BOAT_RADIO_WATCHDOG_NO_CHANGE);
  assert(watchdog.update(false,true)==BOAT_RADIO_WATCHDOG_RESTORED);

  BoatLinkSafetyInputs in={false,false,false,false,false,false,false};
  assert(boatEvaluateLinkFailsafe(in)==BOAT_FS_STOP_ALL);
  // Good GNSS/autopilot data must not convert total radio loss into RTH.
  in.autonomousMissionActive=true; in.gnssValidFresh=true; in.headingValid=true;
  in.batteryHealthy=true; in.geofenceValid=true;
  assert(boatEvaluateLinkFailsafe(in)==BOAT_FS_STOP_ALL);
  in.nrfValidFresh=true;
  assert(boatEvaluateLinkFailsafe(in)==BOAT_FS_NONE);
  in.nrfValidFresh=false; in.loraValidFresh=true;
  // A fresh backup link keeps the radio watchdog alive, but not manual control.
  assert(boatEvaluateLinkFailsafe(in)==BOAT_FS_NONE);
  return 0;
}
