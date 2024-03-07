#include "server.h"
#include "user/nameserver.h"
#include "user/io-server/interface.h"
#include "user/io-server/io_marklin.h"

int get_switch_index(int switch_id)
{
  if (1 <= switch_id && switch_id <= 18) {
      return switch_id-1;
  }
  if (153 <= switch_id && switch_id <= 156) {
      return switch_id-135;
  }
  LOG_ERROR("[SwitchServer ERROR]: Invalid switch id on get index: %d", switch_id);
  return -1;
}

void
unblock_on_switch(LList* switch_requests, int switch_id, SwitchMode mode)
{
    LListIter *it = llist_iter(switch_requests);
    
    while (it->current) {
      SwitchBufferRequest* request = (SwitchBufferRequest *)llist_next(it);
      if (request->switch_id == switch_id || request->switch_id == -1) {
        LOG_DEBUG("[SwitchServer INFO] unblocking task %d on switch %d", request->tid, request->switch_id);
        SwitchResponse response = (SwitchResponse) {
          .type = SWITCH_WAIT,
          .switch_id = switch_id,
          .mode = mode,
        };
        Reply(request->tid, (char *)&response, sizeof(SwitchResponse));
        llist_remove_item(switch_requests, request);
        free(request, SWITCH_BUFFER_REQUEST);
      }
    }
}

void SwitchServer() {
  RegisterAs(SwitchAddress);
  alloc_init(SWITCH_BUFFER_REQUEST, sizeof(SwitchBufferRequest));

  int marklin_io = WhoIs(MarklinIOAddress);
  LList *requests_queue = llist_new();

  SwitchMode switch_states[SWITCH_COUNT];
  for (int i = 0; i < SWITCH_COUNT; i++) {
    switch_states[i] = SWITCH_MODE_UNKNOWN;
  }

  SwitchRequest request;
  SwitchResponse response;
  int from_tid;

  while (1) {
    int req_len = Receive(&from_tid, (char *)&request, sizeof(SwitchRequest));
    if (req_len < 0) {
      LOG_ERROR("[SwitchServer ERROR]: error on receive: %d", req_len);
      continue;
    }

    switch (request.type) {
      case SWITCH_SET:
      {
        response = (SwitchResponse) {
          .type = SWITCH_SET,
          .switch_id = request.switch_id,
          .mode = request.mode
        };
        Reply(from_tid, (char *)&response, sizeof(SwitchResponse));

        if (request.mode == SWITCH_MODE_UNKNOWN) {
          LOG_WARN("[SwitchServer WARNING]: Tried to change switch %d to SWITCH_MODE_UNKNOWN", request.switch_id);
          continue;
        }

        int switch_id = request.switch_id;
        SwitchMode mode = request.mode;
        int switch_index = get_switch_index(switch_id);

        switch_states[switch_index] = mode;
        io_marklin_set_switch(marklin_io, switch_id, mode);
        unblock_on_switch(requests_queue, switch_id, mode);

        // Handle Special Center Case
        if (switch_id == 153 || switch_id == 154) {
          int other_switch = switch_id == 153 ? 154 : 153;
          SwitchMode other_mode = mode == SWITCH_MODE_S ? SWITCH_MODE_C : SWITCH_MODE_S;
          int other_switch_index = get_switch_index(other_switch);
          if (switch_states[other_switch_index] != other_mode) {
            switch_states[other_switch_index] = other_mode;
            io_marklin_set_switch(marklin_io, other_switch, other_mode);
            unblock_on_switch(requests_queue, other_switch, other_mode);
          }
        }

        if (switch_id == 155 || switch_id == 156) {
          int other_switch = switch_id == 155 ? 156 : 155;
          SwitchMode other_mode = mode == SWITCH_MODE_S ? SWITCH_MODE_C : SWITCH_MODE_S;
          int other_switch_index = get_switch_index(other_switch);
          if (switch_states[other_switch_index] != other_mode) {
            switch_states[other_switch_index] = other_mode;
            io_marklin_set_switch(marklin_io, other_switch, other_mode);
            unblock_on_switch(requests_queue, other_switch, other_mode);
          }
        }

        break;
      }
      case SWITCH_GET:
      {
        response = (SwitchResponse) {
          .type = SWITCH_GET,
          .switch_id = request.switch_id,
          .mode = switch_states[get_switch_index(request.switch_id)]
        };
        Reply(from_tid, (char *)&response, sizeof(SwitchResponse));
        break;
      }
      case SWITCH_WAIT:
      {
        SwitchBufferRequest *new_request = alloc(SWITCH_BUFFER_REQUEST);
        new_request->switch_id = request.switch_id;
        new_request->tid = from_tid;
        llist_append(requests_queue, new_request);
        break;
      } 
      default:
      {
        LOG_ERROR("[SwitchServer ERRROR]: Unhandled Switch Request Type - %d", request.type);
        break;
      }
    }
  }

}
