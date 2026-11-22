#include "../source/boat_command_queue.h"
#include <assert.h>

static BoatCommand make(uint16_t id) {
  BoatCommand c={}; c.commandId=id; c.type=CMD_SET_MODE; c.mode=MODE_RETURN_HOME; return c;
}
int main() {
  BoatCommandQueue q;
  BoatCommand c={}; bool dup=false; uint8_t link=0,epoch=0;
  assert(q.size()==0 && !q.full());
  assert(!q.contains(2,11));
  assert(!q.pop(c,dup,link,epoch));
  assert(q.push(make(11),1,false,2));
  assert(q.push(make(12),2,true,2));
  assert(q.push(make(13),1,false,3));
  assert(q.push(make(14),2,false,2));
  assert(q.full() && q.size()==4);
  assert(q.contains(2,11) && q.contains(2,12) && !q.contains(2,13));
  assert(!q.push(make(15),1,false,2));
  assert(q.pop(c,dup,link,epoch));
  assert(c.commandId==11 && link==1 && !dup && epoch==2);
  assert(!q.contains(2,11));
  assert(q.push(make(15),1,false,2));
  assert(q.size()==4);
  assert(q.pop(c,dup,link,epoch) && c.commandId==12 && dup);
  assert(q.pop(c,dup,link,epoch) && c.commandId==13 && epoch==3);
  assert(q.pop(c,dup,link,epoch) && c.commandId==14);
  assert(q.pop(c,dup,link,epoch) && c.commandId==15);
  assert(q.size()==0 && !q.full());
  q.push(make(99),1,false,9); q.clear();
  assert(q.size()==0 && !q.contains(9,99));

  BoatCommand stop=make(201);stop.type=CMD_EMERGENCY_STOP;
  assert(q.push(make(202),1,false,10));
  assert(q.push(stop,1,false,10));
  assert(q.push(make(203),2,false,10));
  q.discardLink(1,true);
  assert(q.size()==2);
  assert(q.pop(c,dup,link,epoch)&&c.commandId==201&&c.type==CMD_EMERGENCY_STOP);
  assert(q.pop(c,dup,link,epoch)&&c.commandId==203);
  assert(q.push(make(204),2,false,10));
  assert(q.push(stop,2,false,10));
  q.clearNonEmergency();
  assert(q.size()==1);
  assert(q.pop(c,dup,link,epoch)&&c.type==CMD_EMERGENCY_STOP);
  return 0;
}
