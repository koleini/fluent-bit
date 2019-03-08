/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Fluent Bit
 *  ==========
 *  Copyright (C) 2019      The Fluent Bit Authors
 *  Copyright (C) 2015-2018 Treasure Data Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <fluent-bit/stream_processor/flb_sp.h>

void sp_clear_window(struct flb_sp *sp)
{
    struct mk_list *head;
    struct flb_sp_record *record;

    mk_list_foreach(head, &sp->window.records) {
        record = mk_list_entry(head, struct flb_sp_record, _head);
        msgpack_unpacked_destroy(record->record);
        flb_free(record);
    }

    mk_list_init(&sp->window.records);
}

int sp_populate_window(struct flb_sp *sp, char *buf_data, size_t buf_size)
{
    int ok;
    msgpack_unpacked *result;
    msgpack_object *obj;
    size_t off;
    struct flb_time tm;
    struct flb_sp_record *record;

    /* vars initialization */
    ok = MSGPACK_UNPACK_SUCCESS;
    off = 0;

    /* Iterate incoming records */
    while (true) {
        result = flb_malloc(sizeof(struct msgpack_unpacked));
        if (!result) {
            flb_errno();
            return -1;
        }

        msgpack_unpacked_init(result);

        if (msgpack_unpack_next(result, buf_data, buf_size, &off) != ok) {
            break;
        }

        flb_time_pop_from_msgpack(&tm, result, &obj);

        record = flb_malloc(sizeof(struct flb_sp_record));
        if (!record) {
            flb_errno();
            return -1;
        }

        record->record = result;
        record->tms = tm;

        mk_list_add(&record->_head, &sp->window.records);
    }

    return 0;
}
