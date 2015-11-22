/*
 * CS492 development module
 * Authors: 
 *  Daniel Berliner dberli2@illinois.edu
 *
 * Description:
 *  This is a test module for the team to do memory tests on.
 *  The final product will be based on code designed here.
 */

#include <sys/types.h>
#include <sys/module.h>
#include <sys/systm.h>  /* uprintf */
#include <sys/param.h>  /* defines used in kernel.h */
#include <sys/kernel.h> /* types used in module initialization */
#include <sys/conf.h>   /* cdevsw struct */
#include <sys/uio.h>    /* uio struct */
#include <sys/malloc.h>

#define BUFFERSIZE 255

/* Function prototypes */
static d_open_t      echo_open;
static d_close_t     echo_close;
static d_read_t      echo_read;
static d_write_t     echo_write;

/* Character device entry points */
static struct cdevsw echo_cdevsw = {
  .d_version = D_VERSION,
  .d_open = echo_open,
  .d_close = echo_close,
  .d_read = echo_read,
  .d_write = echo_write,
  .d_name = "echo",
};


/* vars */
static struct cdev *echo_dev;
static const size_t PAYLOAD_LEN=2;
static char payload[PAYLOAD_LEN];

MALLOC_DECLARE(M_ECHOBUF);
MALLOC_DEFINE(M_ECHOBUF, "echobuffer", "buffer for echo module");

/*
 * This function is called by the kld[un]load(2) system calls to
 * determine what actions to take when a module is loaded or unloaded.
 */
static int
echo_loader(struct module *m __unused, int what, void *arg __unused)
{
  int error = 0;

  switch (what) {
  case MOD_LOAD:                /* kldload */
    error = make_dev_p(MAKEDEV_CHECKNAME | MAKEDEV_WAITOK,
        &echo_dev,
        &echo_cdevsw,
        0,
        UID_ROOT,
        GID_WHEEL,
        0600,
        "echo");
    if (error != 0)
      break;
    printf("Echo device loaded.\n");
    break;
  case MOD_UNLOAD:
    destroy_dev(echo_dev);
    printf("Echo device unloaded.\n");
    break;
  default:
    error = EOPNOTSUPP;
    break;
  }
  return (error);
}

static int
echo_open(struct cdev *dev __unused, int oflags __unused, int devtype __unused,
    struct thread *td __unused)
{
  int error = 0;

  uprintf("Opened device \"echo\" successfully.\n");
  return (error);
}

static int
echo_close(struct cdev *dev __unused, int fflag __unused, int devtype __unused,
    struct thread *td __unused)
{

  uprintf("Closing device \"echo\".\n");
  return (0);
}

/*
 * The read function just takes the buf that was saved via
 * echo_write() and returns it to userland for accessing.
 * uio(9)
 */
static int
echo_read(struct cdev *dev __unused, struct uio *uio, int ioflag __unused)
{
  int error;
  size_t amt=PAYLOAD_LEN;
  //The letter 'W' followed by a NULL
  payload[0] = 'W';
  payload[1] = 0x00;
  amt=MIN(uio->uio_resid, uio->uio_offset >= amt + 1 ? 0 :
      amt + 1 - uio->uio_offset);
  if ((error = uiomove(payload, amt, uio)) != 0)
    uprintf("uiomove failed!\n");

  return (error);
}

/*
 * echo_write takes in a character string and saves it
 * to buf for later accessing.
 */
static int
echo_write(struct cdev *dev __unused, struct uio *uio, int ioflag __unused)
{
  return 0;
}

DEV_MODULE(echo, echo_loader, NULL);