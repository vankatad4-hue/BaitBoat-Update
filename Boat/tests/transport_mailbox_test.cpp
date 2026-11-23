#include "../source/transport_mailbox.h"
#include <cassert>
int main() {
 flytec_mailbox::BoundedMailbox<int,2> q;
 assert(q.capacity()==2 && q.size()==0 && q.dropped()==0);
 int out=0;
 assert(!q.pop(out));
 assert(q.push(1) && q.push(2));
 assert(!q.push(3) && q.dropped()==1 && q.size()==2);
 assert(q.pop(out) && out==1);
 assert(q.push(4));
 assert(q.pop(out) && out==2);
 assert(q.pop(out) && out==4);
 assert(!q.pop(out));
 q.push(5);q.clear();
 assert(q.size()==0 && q.dropped()==1);
 return 0;
}
