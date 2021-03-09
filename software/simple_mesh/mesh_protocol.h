#ifndef MESH_PROTOCOL
#define MESH_PROTOCOL

#include <stdint.h>
#include <string.h>

#include "app_timer.h"
#include "debug_print.h"
#include "ring_buffer.c"
#include "simple_ble.h"
#include "timer.h"

// #define FORMAT1
#define FORMAT2
#define AD_LEN_OFFSET 0
#define AD_MESH_TYPE_OFFSET 1

#define ADV_INTERVAL 50 // min 20 ms
// #define _BLE_ADV_TIMEOUT ((30 + 10) * 10.5 / 10) // units of 10 ms
int BLE_ADV_TIMEOUT = ((ADV_INTERVAL + 10) * 8.5 / 10);
#define MESH_MESSAGE_TIMER_TIMEOUT (BLE_ADV_TIMEOUT * 10 + 30)

#define MAX_REVISION 255
#define NEWER_REVISION(a, b) ((a - b > 0 && a - b < MAX_REVISION / 2) || b - a > MAX_REVISION / 2)

__attribute__((weak)) uint32_t MESH_KEY = 0x42;

typedef struct mesh_data_s {
    uint8_t registers[256][26]; // 0-23: Data, 24: revision, 25: !initial value (0 to start, 1 when first written to)
    queue_t send_queue;
    queue_t request_queue;
} mesh_data_t;

static mesh_data_t m_mesh_data = {
    .registers = { { 0 } },
    .send_queue = {
        .data = { 0 },
        .ptr = 0 },
    .request_queue = { .data = { 0 }, .ptr = 0 }
};

#ifdef FORMAT2
#define MESH_ID_OFFSET 2
#define REQ_FLAG_OFFSET 3
#define REG_NUM_OFFSET 4
#define REG_REVISION_OFFSET 5

// Byte 6 RFU

#define REG_DATA_OFFSET 7

// Intervals for advertising and connections

int TX_POWER_LEVEL = -40;
int APP_ADV_INTERVAL = MSEC_TO_UNITS(ADV_INTERVAL, UNIT_0_625_MS);

#ifndef DEVICE_ID
#define DEVICE_ID 0xDEAD
#error "DEVICE_ID not defined"
#endif

static uint8_t ble_data[31] = { 0 };

static uint8_t m_broadcasting = 0;

static uint16_t drops = 0;
static uint16_t recv_count = 0;

static simple_ble_config_t ble_config = {
    // BLE address is c0:98:e5:4e:00:02
    .platform_id = 0x4E, // used as 4th octet in device BLE address
    .device_id = DEVICE_ID, // used as the 5th and 6th octet in the device BLE address, you will need to change this for each device you have
    .adv_name = "MESH",
    .adv_interval = MSEC_TO_UNITS(1000, UNIT_0_625_MS), // send a packet once per second (minimum is 20 ms)
    .min_conn_interval = MSEC_TO_UNITS(500, UNIT_1_25_MS), // irrelevant if advertising only
    .max_conn_interval = MSEC_TO_UNITS(1000, UNIT_1_25_MS), // irrelevant if advertising only
};
simple_ble_app_t* simple_ble_app;

static int8_t mesh_copy_message(uint8_t* dest, uint8_t reg_num)
{
    // len (30)         0
    // Mesh AD (0x2A)   1
    // MESH_ADDRESS     2
    // Request?         3
    // Reg_num          4
    // reg_revision     5
    // RFU / unused     6
    // Data             7:31

    uint8_t tmp_header[7] = {0};
    // {
    //     30,
    //     0x2A,
    //     MESH_KEY,
    //     0,
    //     reg_num,
    //     m_mesh_data.registers[reg_num][24],
    //     0
    // };
    tmp_header[0] = 30;
    tmp_header[1] = 0x2A;
    tmp_header[MESH_ID_OFFSET] = MESH_KEY;
    tmp_header[REQ_FLAG_OFFSET] = 0;
    tmp_header[REG_NUM_OFFSET] = reg_num;
    tmp_header[REG_REVISION_OFFSET] = m_mesh_data.registers[reg_num][24];

    memcpy(dest, tmp_header, 7);
    memcpy(dest + 7, m_mesh_data.registers[reg_num], 24);
    return 0;
}

static int8_t mesh_copy_request(uint8_t* dest, uint8_t reg_num)
{
    // len (30)         0
    // Mesh AD (0x2A)   1
    // MESH_ADDRESS     2
    // is request       3
    // Reg_num          4
    // reg_revision     5
    // RFU / unused     6
    // Data             7:31

    uint8_t tmp_header[7] = {
        30,
        0x2A,
        MESH_KEY,
        1,
        reg_num,
        m_mesh_data.registers[reg_num][24],
        0
    };
    memcpy(dest, tmp_header, 7);
    memcpy(dest + 7, m_mesh_data.registers[reg_num], 24);
    return 0;
}

void mesh_init()
{
    simple_ble_app = simple_ble_init(&ble_config);
    timers_init();
    printf("Can send a message every %d ms, %d\n", MESH_MESSAGE_TIMER_TIMEOUT, BLE_ADV_TIMEOUT);
    mesh_timer_start(MESH_MESSAGE_TIMER_TIMEOUT);
    mesh_copy_message(ble_data, 0);
    simple_ble_adv_raw(ble_data, 31);
    advertising_start();
    scanning_start();
}

int8_t mesh_assemble_next_message(uint8_t* dest)
{
    DEBUG_PRINT("Assembling. Here is send_queue:\n\t");
    q_print_status(&(m_mesh_data.send_queue));
    if (q_has_data(&(m_mesh_data.request_queue))) {
        uint8_t reg_num = q_pop(&(m_mesh_data.request_queue));
        mesh_copy_request(dest, reg_num);
        return 0;
    } else if (q_has_data(&(m_mesh_data.send_queue))) {
        uint8_t reg_num = q_pop(&(m_mesh_data.send_queue));
        mesh_copy_message(dest, reg_num);
        return 0;
    }
    return -1;
}

static void broadcast_next_message()
{
    if (mesh_assemble_next_message(ble_data) == 0) {
        DEBUG_PRINT("about to %s reg %d = %d @ t=%ld\n", ble_data[REQ_FLAG_OFFSET] == 1 ? "request" : "send", ble_data[REG_NUM_OFFSET], ble_data[30], app_timer_cnt_get());
        m_broadcasting = 1;
        advertising_stop();
        copy_adv_data(ble_data, 31);
        advertising_start();
        mesh_timer_start(MESH_MESSAGE_TIMER_TIMEOUT);
    } else {
        DEBUG_PRINT("not sending yet\n");
        m_broadcasting = 0;
    }
}

void mesh_timer_timeout_handler(void* p_context)
{
    UNUSED_PARAMETER(p_context);
    broadcast_next_message();
}

static void print_24_bytes(uint8_t* data)
{
    DEBUG_PRINT("%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX\n",
        data[0], data[1], data[2], data[3],
        data[4], data[7], data[6], data[7],
        data[8], data[9], data[10], data[11],
        data[12], data[13], data[14], data[15],
        data[16], data[17], data[18], data[19],
        data[20], data[21], data[22], data[23]);
}

void mesh_write_reg(uint8_t reg_num, uint8_t* data, uint8_t revision)
{
    print_24_bytes(m_mesh_data.registers[reg_num]);
    print_24_bytes(data);
    m_mesh_data.registers[reg_num][24] = revision;
    m_mesh_data.registers[reg_num][25] = 1;
    memcpy(m_mesh_data.registers[reg_num], data, 24);
    q_push(&(m_mesh_data.send_queue), reg_num);
    DEBUG_PRINT("wrote reg %d = %d @ t=%ld\nbroadcasting currently? %d\n", reg_num, m_mesh_data.registers[reg_num][23], app_timer_cnt_get(), m_broadcasting);

    if (!m_broadcasting) {
        broadcast_next_message();
    }
}

void __attribute__((weak)) mesh_message_recv_callback();

void ble_evt_adv_report(ble_evt_t const* p_ble_evt)
{

    ble_gap_evt_adv_report_t const* adv_report = &(p_ble_evt->evt.gap_evt.params.adv_report);
    // uint8_t const* ble_addr = adv_report->peer_addr.addr; // array of 6 bytes of the address
    uint8_t* adv_buf = adv_report->data.p_data; // array of up to 31 bytes of advertisement payload data
    uint16_t adv_len = adv_report->data.len;
    // if message has the right key, check to update
    // if register is newer than before, update regsiter
    uint16_t device_id = adv_report->peer_addr.addr[0] | (adv_report->peer_addr.addr[1] << 8);
    for (int i = 0; i < 6; i++) {
        device_id |= adv_report->peer_addr.addr[i] << (8 * i);
    }

    if ((adv_len == 31) && (adv_buf[1] == 0x2A) && (adv_buf[MESH_ID_OFFSET] == MESH_KEY)) {
        uint8_t reg_num = adv_buf[REG_NUM_OFFSET];
        if (adv_buf[REQ_FLAG_OFFSET] == 0) {
            uint8_t reg_revision_message = adv_buf[REG_REVISION_OFFSET];
            uint8_t reg_revision_local = m_mesh_data.registers[reg_num][24];
            if (NEWER_REVISION(reg_revision_message, reg_revision_local) || m_mesh_data.registers[reg_num][25] == 0) {
                if (reg_revision_message - reg_revision_local > 1 && (m_mesh_data.registers[reg_num][25] == 1)) {
                    printf("we dropped a message!\n\tTotal drops: %d\n\tTotal recv: %d\n", ++drops, ++recv_count);
                } else {
                    printf("we receieved a message from %lX!\n\tTotal drops: %d\n\tTotal recv: %d\n", device_id, drops, ++recv_count);
                }
                DEBUG_PRINT("it's a newer reg %d version\n", reg_num);
                if (reg_num % 3 == 0 && device_id != 0xFEED || reg_num % 3 == 2 && device_id != 0xCAFE || reg_num % 3 == 1 && device_id != 0xBEEF) {
                    DEBUG_PRINT("but it's from %X\n", device_id);
                }
                mesh_write_reg(reg_num, adv_buf + REG_DATA_OFFSET, reg_revision_message);
                if (mesh_message_recv_callback) {
                    mesh_message_recv_callback();
                }
            }
        } else if (adv_buf[REQ_FLAG_OFFSET] == 1) {
            DEBUG_PRINT("Received a request to send register %d\n", reg_num);
            q_push(&(m_mesh_data.send_queue), reg_num);
            if (!m_broadcasting) {
                broadcast_next_message();
            }
        }
    }
}

void mesh_read_reg(uint8_t* dest, uint8_t reg_nun)
{
    memcpy(dest, m_mesh_data.registers[reg_nun], 24);
}

void mesh_request_reg(uint8_t reg_num)
{
    q_push(&(m_mesh_data.request_queue), reg_num);
    m_mesh_data.registers[reg_num][25] = 0;
    DEBUG_PRINT("pushed request for reg %d\n", reg_num);
    if (!m_broadcasting) {
        broadcast_next_message();
    }
}

#endif

#ifdef FORMAT1
#define MESH_ID_OFFSET 2
#define TTL_OFFSET 3
#define DEST_OFFSET 4
#define SOURCE_OFFSET 5
#define REG_NUM_OFFSET 6
#define REG_DATA_OFFSET 7

__attribute__((weak)) uint8_t device_address = 0;

typedef struct mesh_message_s {
    uint8_t network_key;
    uint8_t ttl;
    uint8_t dest_addr;
    uint8_t source_addr;
    uint8_t reg_num;
} mesh_message_t;

/** Writes a value to the register
 */
void mesh_write_reg(mesh_data_t* mesh_data, uint8_t reg_num, uint8_t* data); // writes to reg

uint8_t* read_reg(mesh_data_t* mesh_data, uint8_t reg_num);

/** Add a register to the broadcast queue to be broadcast
 *  Returns -1 on failure, 0 on success 
 */
int8_t add_to_broadcast_queue(mesh_data_t* mesh_data, uint8_t reg_num);

/** Add a register to the head of the queue
 *  Returns -1 if failure, 0 if success
 */
int8_t broadcast_next(mesh_data_t* mesh_data, uint8_t reg_num);

uint8_t* assemble_message(uint8_t* dest, mesh_data_t* mesh_data, mesh_message_t* mesh_message);

uint8_t message_is_mine(uint8_t* message, )

#endif

#endif