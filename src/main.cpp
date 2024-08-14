// /*   SPDX-License-Identifier: BSD-3-Clause
//  *   Copyright (C) 2017 Intel Corporation.
//  *   All rights reserved.
//  */

#include "spdk/stdinc.h"

#include "spdk/bdev.h"
#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/blob_bdev.h"
#include "spdk/blob.h"
#include "spdk/log.h"
#include "spdk/string.h"

// /*
//  * We'll use this struct to gather housekeeping hello_context to pass between
//  * our events and callbacks.
//  */
// struct hello_context_t {
// 	struct spdk_blob_store *bs;
// 	struct spdk_blob *blob;
// 	spdk_blob_id blobid;
// 	struct spdk_io_channel *channel;
// 	uint8_t *read_buff;
// 	uint8_t *write_buff;
// 	uint64_t io_unit_size;
// 	int rc;
// };

// /*
//  * Free up memory that we allocated.
//  */
// static void
// hello_cleanup(struct hello_context_t *hello_context)
// {
// 	spdk_free(hello_context->read_buff);
// 	spdk_free(hello_context->write_buff);
// 	free(hello_context);
// }

// /*
//  * Callback routine for the blobstore unload.
//  */
// static void
// unload_complete(void *cb_arg, int bserrno)
// {
// 	struct hello_context_t *hello_context = cb_arg;

// 	SPDK_NOTICELOG("entry\n");
// 	if (bserrno) {
// 		SPDK_ERRLOG("Error %d unloading the bobstore\n", bserrno);
// 		hello_context->rc = bserrno;
// 	}

// 	spdk_app_stop(hello_context->rc);
// }

// /*
//  * Unload the blobstore, cleaning up as needed.
//  */
// static void
// unload_bs(struct hello_context_t *hello_context, char *msg, int bserrno)
// {
// 	if (bserrno) {
// 		SPDK_ERRLOG("%s (err %d)\n", msg, bserrno);
// 		hello_context->rc = bserrno;
// 	}
// 	if (hello_context->bs) {
// 		if (hello_context->channel) {
// 			spdk_bs_free_io_channel(hello_context->channel);
// 		}
// 		spdk_bs_unload(hello_context->bs, unload_complete, hello_context);
// 	} else {
// 		spdk_app_stop(bserrno);
// 	}
// }

// /*
//  * Callback routine for the deletion of a blob.
//  */
// static void
// delete_complete(void *arg1, int bserrno)
// {
// 	struct hello_context_t *hello_context = arg1;

// 	SPDK_NOTICELOG("entry\n");
// 	if (bserrno) {
// 		unload_bs(hello_context, "Error in delete completion",
// 			  bserrno);
// 		return;
// 	}

// 	/* We're all done, we can unload the blobstore. */
// 	unload_bs(hello_context, "", 0);
// }

// /*
//  * Function for deleting a blob.
//  */
// static void
// delete_blob(void *arg1, int bserrno)
// {
// 	struct hello_context_t *hello_context = arg1;

// 	SPDK_NOTICELOG("entry\n");
// 	if (bserrno) {
// 		unload_bs(hello_context, "Error in close completion",
// 			  bserrno);
// 		return;
// 	}

// 	spdk_bs_delete_blob(hello_context->bs, hello_context->blobid,
// 			    delete_complete, hello_context);
// }

// /*
//  * Callback function for reading a blob.
//  */
// static void
// read_complete(void *arg1, int bserrno)
// {
// 	struct hello_context_t *hello_context = arg1;
// 	int match_res = -1;

// 	SPDK_NOTICELOG("entry\n");
// 	if (bserrno) {
// 		unload_bs(hello_context, "Error in read completion",
// 			  bserrno);
// 		return;
// 	}

// 	/* Now let's make sure things match. */
// 	match_res = memcmp(hello_context->write_buff, hello_context->read_buff,
// 			   hello_context->io_unit_size);
// 	if (match_res) {
// 		unload_bs(hello_context, "Error in data compare", -1);
// 		return;
// 	} else {
// 		SPDK_NOTICELOG("read SUCCESS and data matches!\n");
// 	}

// 	/* Now let's close it and delete the blob in the callback. */
// 	spdk_blob_close(hello_context->blob, delete_blob, hello_context);
// }

// /*
//  * Function for reading a blob.
//  */
// static void
// read_blob(struct hello_context_t *hello_context)
// {
// 	SPDK_NOTICELOG("entry\n");

// 	hello_context->read_buff = spdk_malloc(hello_context->io_unit_size,
// 					       0x1000, NULL, SPDK_ENV_LCORE_ID_ANY,
// 					       SPDK_MALLOC_DMA);
// 	if (hello_context->read_buff == NULL) {
// 		unload_bs(hello_context, "Error in memory allocation",
// 			  -ENOMEM);
// 		return;
// 	}

// 	/* Issue the read and compare the results in the callback. */
// 	spdk_blob_io_read(hello_context->blob, hello_context->channel,
// 			  hello_context->read_buff, 0, 1, read_complete,
// 			  hello_context);
// }

// /*
//  * Callback function for writing a blob.
//  */
// static void
// write_complete(void *arg1, int bserrno)
// {
// 	struct hello_context_t *hello_context = arg1;

// 	SPDK_NOTICELOG("entry\n");
// 	if (bserrno) {
// 		unload_bs(hello_context, "Error in write completion",
// 			  bserrno);
// 		return;
// 	}

// 	/* Now let's read back what we wrote and make sure it matches. */
// 	read_blob(hello_context);
// }

// /*
//  * Function for writing to a blob.
//  */
// static void
// blob_write(struct hello_context_t *hello_context)
// {
// 	SPDK_NOTICELOG("entry\n");

// 	/*
// 	 * Buffers for data transfer need to be allocated via SPDK. We will
// 	 * transfer 1 io_unit of 4K aligned data at offset 0 in the blob.
// 	 */
// 	hello_context->write_buff = spdk_malloc(hello_context->io_unit_size,
// 						0x1000, NULL, SPDK_ENV_LCORE_ID_ANY,
// 						SPDK_MALLOC_DMA);
// 	if (hello_context->write_buff == NULL) {
// 		unload_bs(hello_context, "Error in allocating memory",
// 			  -ENOMEM);
// 		return;
// 	}
// 	memset(hello_context->write_buff, 0x5a, hello_context->io_unit_size);

// 	/* Now we have to allocate a channel. */
// 	hello_context->channel = spdk_bs_alloc_io_channel(hello_context->bs);
// 	if (hello_context->channel == NULL) {
// 		unload_bs(hello_context, "Error in allocating channel",
// 			  -ENOMEM);
// 		return;
// 	}

// 	/* Let's perform the write, 1 io_unit at offset 0. */
// 	spdk_blob_io_write(hello_context->blob, hello_context->channel,
// 			   hello_context->write_buff,
// 			   0, 1, write_complete, hello_context);
// }

// /*
//  * Callback function for syncing metadata.
//  */
// static void
// sync_complete(void *arg1, int bserrno)
// {
// 	struct hello_context_t *hello_context = arg1;

// 	SPDK_NOTICELOG("entry\n");
// 	if (bserrno) {
// 		unload_bs(hello_context, "Error in sync callback",
// 			  bserrno);
// 		return;
// 	}

// 	/* Blob has been created & sized & MD sync'd, let's write to it. */
// 	blob_write(hello_context);
// }

// static void
// resize_complete(void *cb_arg, int bserrno)
// {
// 	struct hello_context_t *hello_context = cb_arg;
// 	uint64_t total = 0;

// 	if (bserrno) {
// 		unload_bs(hello_context, "Error in blob resize", bserrno);
// 		return;
// 	}

// 	total = spdk_blob_get_num_clusters(hello_context->blob);
// 	SPDK_NOTICELOG("resized blob now has USED clusters of %" PRIu64 "\n",
// 		       total);

// 	/*
// 	 * Metadata is stored in volatile memory for performance
// 	 * reasons and therefore needs to be synchronized with
// 	 * non-volatile storage to make it persistent. This can be
// 	 * done manually, as shown here, or if not it will be done
// 	 * automatically when the blob is closed. It is always a
// 	 * good idea to sync after making metadata changes unless
// 	 * it has an unacceptable impact on application performance.
// 	 */
// 	spdk_blob_sync_md(hello_context->blob, sync_complete, hello_context);
// }

// /*
//  * Callback function for opening a blob.
//  */
// static void
// open_complete(void *cb_arg, struct spdk_blob *blob, int bserrno)
// {
// 	struct hello_context_t *hello_context = cb_arg;
// 	uint64_t free = 0;

// 	SPDK_NOTICELOG("entry\n");
// 	if (bserrno) {
// 		unload_bs(hello_context, "Error in open completion",
// 			  bserrno);
// 		return;
// 	}


// 	hello_context->blob = blob;
// 	free = spdk_bs_free_cluster_count(hello_context->bs);
// 	SPDK_NOTICELOG("blobstore has FREE clusters of %" PRIu64 "\n",
// 		       free);

// 	/*
// 	 * Before we can use our new blob, we have to resize it
// 	 * as the initial size is 0. For this example we'll use the
// 	 * full size of the blobstore but it would be expected that
// 	 * there'd usually be many blobs of various sizes. The resize
// 	 * unit is a cluster.
// 	 */
// 	spdk_blob_resize(hello_context->blob, free, resize_complete, hello_context);
// }

// /*
//  * Callback function for creating a blob.
//  */
// static void
// blob_create_complete(void *arg1, spdk_blob_id blobid, int bserrno)
// {
// 	struct hello_context_t *hello_context = arg1;

// 	SPDK_NOTICELOG("entry\n");
// 	if (bserrno) {
// 		unload_bs(hello_context, "Error in blob create callback",
// 			  bserrno);
// 		return;
// 	}

// 	hello_context->blobid = blobid;
// 	SPDK_NOTICELOG("new blob id %" PRIu64 "\n", hello_context->blobid);

// 	/* We have to open the blob before we can do things like resize. */
// 	spdk_bs_open_blob(hello_context->bs, hello_context->blobid,
// 			  open_complete, hello_context);
// }

// /*
//  * Function for creating a blob.
//  */
// static void
// create_blob(struct hello_context_t *hello_context)
// {
// 	SPDK_NOTICELOG("entry\n");
// 	spdk_bs_create_blob(hello_context->bs, blob_create_complete, hello_context);
// }

// /*
//  * Callback function for initializing the blobstore.
//  */
// static void
// bs_init_complete(void *cb_arg, struct spdk_blob_store *bs,
// 		 int bserrno)
// {
// 	struct hello_context_t *hello_context = cb_arg;

// 	SPDK_NOTICELOG("entry\n");
// 	if (bserrno) {
// 		unload_bs(hello_context, "Error initing the blobstore",
// 			  bserrno);
// 		return;
// 	}

// 	hello_context->bs = bs;
// 	SPDK_NOTICELOG("blobstore: %p\n", hello_context->bs);
// 	/*
// 	 * We will use the io_unit size in allocating buffers, etc., later
// 	 * so we'll just save it in out context buffer here.
// 	 */
// 	hello_context->io_unit_size = spdk_bs_get_io_unit_size(hello_context->bs);

// 	/*
// 	 * The blobstore has been initialized, let's create a blob.
// 	 * Note that we could pass a message back to ourselves using
// 	 * spdk_thread_send_msg() if we wanted to keep our processing
// 	 * time limited.
// 	 */
// 	create_blob(hello_context);
// }

// static void
// base_bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev,
// 		   void *event_ctx)
// {
// 	SPDK_WARNLOG("Unsupported bdev event: type %d\n", type);
// }

// /*
//  * Our initial event that kicks off everything from main().
//  */
// static void
// hello_start(void *arg1)
// {
// 	struct hello_context_t *hello_context = arg1;
// 	struct spdk_bs_dev *bs_dev = NULL;
// 	int rc;

// 	SPDK_NOTICELOG("entry\n");

// 	/*
// 	 * In this example, use our malloc (RAM) disk configured via
// 	 * hello_blob.json that was passed in when we started the
// 	 * SPDK app framework.
// 	 *
// 	 * spdk_bs_init() requires us to fill out the structure
// 	 * spdk_bs_dev with a set of callbacks. These callbacks
// 	 * implement read, write, and other operations on the
// 	 * underlying disks. As a convenience, a utility function
// 	 * is provided that creates an spdk_bs_dev that implements
// 	 * all of the callbacks by forwarding the I/O to the
// 	 * SPDK bdev layer. Other helper functions are also
// 	 * available in the blob lib in blob_bdev.c that simply
// 	 * make it easier to layer blobstore on top of a bdev.
// 	 * However blobstore can be more tightly integrated into
// 	 * any lower layer, such as NVMe for example.
// 	 */
// 	rc = spdk_bdev_create_bs_dev_ext("Malloc0", base_bdev_event_cb, NULL, &bs_dev);
// 	if (rc != 0) {
// 		SPDK_ERRLOG("Could not create blob bdev, %s!!\n",
// 			    spdk_strerror(-rc));
// 		spdk_app_stop(-1);
// 		return;
// 	}

// 	spdk_bs_init(bs_dev, NULL, bs_init_complete, hello_context);
// }

// int
// main(int argc, char **argv)
// {
// 	struct spdk_app_opts opts = {};
// 	int rc = 0;
// 	struct hello_context_t *hello_context = NULL;

// 	SPDK_NOTICELOG("entry\n");

// 	/* Set default values in opts structure. */
// 	spdk_app_opts_init(&opts, sizeof(opts));

// 	/*
// 	 * Setup a few specifics before we init, for most SPDK cmd line
// 	 * apps, the config file will be passed in as an arg but to make
// 	 * this example super simple we just hardcode it. We also need to
// 	 * specify a name for the app.
// 	 */
// 	opts.name = "hello_blob";
// 	opts.json_config_file = argv[1];
// 	opts.rpc_addr = NULL;

// 	/*
// 	 * Now we'll allocate and initialize the blobstore itself. We
// 	 * can pass in an spdk_bs_opts if we want something other than
// 	 * the defaults (cluster size, etc), but here we'll just take the
// 	 * defaults.  We'll also pass in a struct that we'll use for
// 	 * callbacks so we've got efficient bookkeeping of what we're
// 	 * creating. This is an async operation and bs_init_complete()
// 	 * will be called when it is complete.
// 	 */
// 	hello_context = calloc(1, sizeof(struct hello_context_t));
// 	if (hello_context != NULL) {
// 		/*
// 		 * spdk_app_start() will block running hello_start() until
// 		 * spdk_app_stop() is called by someone (not simply when
// 		 * hello_start() returns), or if an error occurs during
// 		 * spdk_app_start() before hello_start() runs.
// 		 */
// 		rc = spdk_app_start(&opts, hello_start, hello_context);
// 		if (rc) {
// 			SPDK_NOTICELOG("ERROR!\n");
// 		} else {
// 			SPDK_NOTICELOG("SUCCESS!\n");
// 		}
// 		/* Free up memory that we allocated */
// 		hello_cleanup(hello_context);
// 	} else {
// 		SPDK_ERRLOG("Could not alloc hello_context struct!!\n");
// 		rc = -ENOMEM;
// 	}

// 	/* Gracefully close out all of the SPDK subsystems. */
// 	spdk_app_fini();
// 	return rc;
// }



#include "core.hpp"

using namespace sqk;

void bdev_create_wakeup_fn(enum spdk_bdev_event_type type,
                           struct spdk_bdev *bdev, void *waker) {
    S_INFO("bdev_create_wakeup_fn type: {}", (int)type);
}
void init_waker_fn(void *cb_arg, struct spdk_blob_store *bs,
		int bserrno) {
    assert(bserrno == 0);
    auto waker = static_cast<Awaker<spdk_blob_store*>*>(cb_arg);
    waker->wake(std::move(bs));
}
void blob_wake_fn(void *cb_arg, spdk_blob_id blobid, int bserrno) {
    assert(bserrno == 0);
    auto waker = static_cast<Awaker<spdk_blob_id>*>(cb_arg);
    waker->wake(std::move(blobid));
}

static void
open_cb(void *cb_arg, struct spdk_blob *blob, int bserrno)
{

    assert(bserrno == 0);
    auto waker = static_cast<Awaker<spdk_blob*>*>(cb_arg);
    waker->wake(std::move(blob));
}
static void
resize_cb(void *cb_arg, int bserrno)
{
    assert(bserrno == 0);
    auto waker = static_cast<Awaker<void>*>(cb_arg);
    waker->wake();
}

void wakeup_fn(int rc, void *cb_arg) {
    assert(!rc);
    auto waker = static_cast<Awaker<void>*>(cb_arg);
    waker->wake();
}

const char *
dpdk_cli_override_opts = "--log-level=lib.eal:4 "
			 "--log-level=lib.malloc:4 "
			 "--log-level=lib.ring:4 "
			 "--log-level=lib.mempool:4 "
			 "--log-level=lib.timer:4 "
			 "--log-level=pmd:4 "
			 "--log-level=lib.hash:4 "
			 "--log-level=lib.lpm:4 "
			 "--log-level=lib.kni:4 "
			 "--log-level=lib.acl:4 "
			 "--log-level=lib.power:4 "
			 "--log-level=lib.meter:4 "
			 "--log-level=lib.sched:4 "
			 "--log-level=lib.port:4 "
			 "--log-level=lib.table:4 "
			 "--log-level=lib.pipeline:4 "
			 "--log-level=lib.mbuf:4 "
			 "--log-level=lib.cryptodev:4 "
			 "--log-level=lib.efd:4 "
			 "--log-level=lib.eventdev:4 "
			 "--log-level=lib.gso:4 "
			 "--log-level=user1:4 "
			 "--log-level=user2:4 "
			 "--log-level=user3:4 "
			 "--log-level=user4:4 "
			 "--log-level=user5:4 "
			 "--log-level=user6:4 "
			 "--log-level=user7:4 "
			 "--log-level=user8:4 "
			 "--no-telemetry";

Task<int> run() {
    S_INFO("run");
  int rc;
  spdk_bs_dev *bs_dev;
  Awaker<void> waker{};

  spdk_env_opts opts{};
  spdk_env_opts_init(&opts);
  opts.env_context = (char *)dpdk_cli_override_opts;
  opts.name = "hello";
  rc = spdk_env_init(&opts);
  assert(!rc);

  // rc = spdk_thread_lib_init(NULL, 0);
  // assert(!rc);
  spdk_log_set_print_level(SPDK_LOG_DEBUG);
  spdk_log_set_level(SPDK_LOG_DEBUG);

  S_INFO("before start");
  spdk_thread *thread = spdk_thread_create("spdk_thread", NULL);
  assert(thread);
  spdk_set_thread(thread);

  spdk_subsystem_init_from_json_config(
      "../src/hello_blob.json", SPDK_DEFAULT_RPC_ADDR, wakeup_fn, &waker, true);

  scheduler->run([](spdk_thread *thr) -> Task<void> {
    while (true) {
      spdk_thread_poll(thr, 0, 0);
      co_yield nullptr;
    }
    co_return;
  }(thread));

  co_await waker;
  S_INFO("after start");

  S_INFO("before create");
  rc = spdk_bdev_create_bs_dev_ext("Malloc0", bdev_create_wakeup_fn, NULL,
                                   &bs_dev);
  assert(!rc);
  S_INFO("after create");

  S_INFO("before init");
  Awaker<spdk_blob_store *> waker2{};
  spdk_bs_init(bs_dev, NULL, init_waker_fn, &waker2);
  auto bs = co_await waker2;
  int io_unit_size = spdk_bs_get_io_unit_size(bs);
  S_INFO("after init");

  S_INFO("before blob");
  Awaker<spdk_blob_id> waker3{};
  spdk_bs_create_blob(bs, blob_wake_fn, &waker3);
  spdk_blob_id id = co_await waker3;
  S_INFO("after blob");

  Awaker<spdk_blob *> waker4;
  spdk_bs_open_blob(bs, id, open_cb, &waker4);
  auto blob = co_await waker4;

  uint64_t free = spdk_bs_free_cluster_count(bs);
  spdk_blob_resize(blob, free, resize_cb, &waker);
  uint64_t total = spdk_blob_get_num_clusters(blob);

  spdk_blob_sync_md(blob, resize_cb, &waker);
  uint8_t *write_buff = (uint8_t *)spdk_malloc(
      io_unit_size, 0x1000, NULL, SPDK_ENV_LCORE_ID_ANY, SPDK_MALLOC_DMA);
  assert(write_buff != nullptr);
  memset(write_buff, 0x5a, io_unit_size);
  spdk_io_channel *channel = spdk_bs_alloc_io_channel(bs);

  spdk_blob_io_write(blob, channel, write_buff, 0, 1, resize_cb, &waker);
  co_await waker;

  uint8_t *read_buff = (uint8_t *)spdk_malloc(
      io_unit_size, 0x1000, NULL, SPDK_ENV_LCORE_ID_ANY, SPDK_MALLOC_DMA);

  spdk_blob_io_read(blob, channel, read_buff, 0, 1, resize_cb, &waker);
  co_await waker;

  int match_res = memcmp(write_buff, read_buff, io_unit_size);

  if (match_res) {
    S_WARN("data not matches!!!");
  } else {
    S_INFO("data read complete and matches!!!");
  }
  co_return 0;
}

int main (int argc, char *argv[]) {
    S_LOGGER_SETUP;
    S_INFO("start....");


        scheduler = new SQKScheduler;
    scheduler->run(run());

}
