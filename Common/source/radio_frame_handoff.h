#pragma once
#include "radio_protocol.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <atomic>
#include <type_traits>
// Dormant R30H-99: raw frames are NOT authenticated commands.
// One owner per direction; begin/end only while both workers stopped.
namespace flytec_rtos {
enum class PhyLink:uint8_t { Primary=1, Backup=2 };
struct RxFrame { RadioFrame frame; uint32_t receivedMs; PhyLink link; };
// Local owner metadata only; not part of the authenticated 32-byte RF frame.
enum class TxTraffic:uint8_t { Unclassified, Routine, BackupMission, EmergencyStop };
struct TxRequest { RadioFrame frame; uint32_t ticket; uint32_t generation; PhyLink link; uint32_t createdMs=0; uint32_t lifetimeMs=0; TxTraffic traffic=TxTraffic::Unclassified; };
enum class TxEvidence:uint8_t { None, NrfAck, LocalReady };
struct TxCompletion {
 uint32_t ticket; uint32_t generation; PhyLink link; bool success; TxEvidence evidence;
 TxCompletion(uint32_t t=0,uint32_t g=0,PhyLink l=PhyLink::Primary,bool ok=false,TxEvidence e=TxEvidence::None):
 ticket(t),generation(g),link(l),success(ok),evidence(e){}
};
template<class T,unsigned Depth> class FrameQueue {
 static_assert(Depth>0,"Queue depth required");
 static_assert(std::is_trivially_copyable<T>::value,"Copyable records required");
public:
 FrameQueue()=default;
 FrameQueue(const FrameQueue&)=delete;
 FrameQueue& operator=(const FrameQueue&)=delete;
 bool begin(){if(!q_)q_=xQueueCreate(Depth,sizeof(T));return q_!=nullptr;}
 ~FrameQueue(){if(q_)vQueueDelete(q_);}
 bool offer(const T& item){
  if(q_&&xQueueSend(q_,&item,0)==pdTRUE)return true;
  overflow_.store(true);return false;
 }
 bool take(T& item){return q_&&xQueueReceive(q_,&item,0)==pdTRUE;}
 bool fault()const{return overflow_.load();}
private:
 QueueHandle_t q_=nullptr;
 std::atomic<bool> overflow_{false};
};
struct RadioFrameHandoff {
 FrameQueue<RxFrame,8> primaryRx,backupRx;
 FrameQueue<TxRequest,4> primaryTx,backupTx;
 // Critical classification is supplied only by the authenticated application.
 FrameQueue<TxRequest,2> primaryCritical,backupCritical;
 FrameQueue<TxCompletion,8> primaryCompletions,backupCompletions;
 // A malformed link identity must not redirect a completion into the other radio.
 bool offerCompletion(PhyLink link,const TxCompletion& c){
  if(link==PhyLink::Primary)return primaryCompletions.offer(c);
  if(link==PhyLink::Backup)return backupCompletions.offer(c);
  return false;
 }
 bool takeCompletion(PhyLink link,TxCompletion& c){
  if(link==PhyLink::Primary)return primaryCompletions.take(c);
  if(link==PhyLink::Backup)return backupCompletions.take(c);
  return false;
 }
 bool fault(PhyLink link)const{
  if(link==PhyLink::Primary)return primaryRx.fault()||primaryTx.fault()||primaryCritical.fault()||primaryCompletions.fault();
  if(link==PhyLink::Backup)return backupRx.fault()||backupTx.fault()||backupCritical.fault()||backupCompletions.fault();
  return true;
 }
 bool takeTx(PhyLink link,TxRequest& out){
  if(link==PhyLink::Primary)return primaryCritical.take(out)||primaryTx.take(out);
  if(link==PhyLink::Backup)return backupCritical.take(out)||backupTx.take(out);
  return false;
 }
 bool begin(){return primaryRx.begin()&&backupRx.begin()&&primaryTx.begin()&&backupTx.begin()&&primaryCritical.begin()&&backupCritical.begin()&&primaryCompletions.begin()&&backupCompletions.begin();}
 bool fault()const{return fault(PhyLink::Primary)||fault(PhyLink::Backup);}
};
}
