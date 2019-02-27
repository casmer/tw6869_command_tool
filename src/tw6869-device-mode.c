


//The headers
#include <ecode.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h> /* getopt_long() */

#include <fcntl.h> /* low-level i/o */

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <glib.h>
#include <linux/param.h>

#include <asm/types.h> /* for videodev2.h */

#include <linux/videodev2.h>

#define TW6869_HW_RESET_IOCTL BASE_VIDIOC_PRIVATE + 1
#define TW6869_HW_RESET_SET_DELAY_IOCTL BASE_VIDIOC_PRIVATE + 2

#define CLEAR(x) memset (&(x), 0, sizeof (x))



struct my_device_info {

    gchar *dev_name;          /* Video in device */
    gint fd;
    gint field;
    gint hw_reset;
    uint hw_reset_delay_ms;
};


static unsigned int g_dbg = 0;

#define dbg(lvl, fmt, ...) _dbg (__func__, __LINE__, lvl, fmt "\n", ##__VA_ARGS__)
void _dbg(const char *func, unsigned int line,
      unsigned int lvl, const char *fmt, ...)
{
    if (g_dbg >= lvl) {
        va_list ap;
        printf("[%d]:%s:%d - ", lvl, func, line);
        va_start(ap, fmt);
        vprintf(fmt, ap);
        fflush(stdout);
        va_end(ap);
    }
}

static struct my_device_info info = {
        .dev_name = "/dev/video0",
        .fd = -1,
        .field = 0,
        .hw_reset = FALSE,
        .hw_reset_delay_ms = 0
};


static void open_device (void)
{
    info.fd = open (info.dev_name, O_RDWR | O_NONBLOCK, 0);
    if (-1 == info.fd)
    {
        fprintf (stderr, "Cannot open %s: %d, %s\n",info.dev_name,errno, strerror (errno));
        exit (EXIT_FAILURE);
    }
}
static void close_device (void)
{
    if (-1 == close (info.fd))
        {
            exit(-1);
        }
    info.fd = -1;
}


void print_device_info(struct v4l2_format fmt)
{
    printf("Device Info for %s:\n", info.dev_name);
    printf("\t field: %d\n", fmt.fmt.pix.field);
    printf("\t width: %d\n", fmt.fmt.pix.width);
    printf("\t height: %d\n", fmt.fmt.pix.height);

    char st[5] = {0};
    st[0] = fmt.fmt.pix.pixelformat & 0xff;
    st[1] = (fmt.fmt.pix.pixelformat >> 8) & 0xff;
    st[2] = (fmt.fmt.pix.pixelformat >> 16) & 0xff;
    st[3] = (fmt.fmt.pix.pixelformat >> 24) & 0xff;
    printf("\t pixelformat: %s\n", st);
}

int main (int argc, char *argv[])
{


    const struct option long_opts[] = {
        {"help",                   no_argument,       0, '?'},
        {"video-in",               required_argument, 0, 'i'},
        {"field",               required_argument, 0, 'f'},
        {"hw-rst",               no_argument, 0, 'r'},
        {"hw-rst-delay",               required_argument, 0, 'd'},
        { /* Sentinel */ }
    };
    char *arg_parse = "?hri:f:d:";
     char *usage = "learn the code!\n";

    /* Parse Args */
        while (TRUE) {
            int opt_ndx;
            int c = getopt_long(argc, argv, arg_parse, long_opts, &opt_ndx);

            if (c < 0)
                break;

            switch (c) {
            case 0: /* long-opts only parsing */
                if (strcmp(long_opts[opt_ndx].name, "help") == 0) {
                    /* Change steps to internal usage of it */
                    puts(usage);
                    return -ECODE_ARGS;
                } else {
                    puts(usage);
                    return -ECODE_ARGS;
                }
                break;
            case 'f':
                info.field =atoi( optarg);
                dbg(1, "set video in to: %s", info.dev_name);
                break;
            case 'i': /* Video in parameter */
                info.dev_name = optarg;
                dbg(1, "set video in to: %s", info.dev_name);
                break;
            case 'r':
                info.hw_reset = TRUE;
                break;
            case 'd':
                info.hw_reset_delay_ms = atoi(optarg);
                break;
            default: /* Default - bad arg */
                puts(usage);
                return -ECODE_ARGS;
            }
        }

        open_device();
        struct v4l2_format fmt1, fmt;
        fmt1.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(info.fd, VIDIOC_G_FMT, &fmt1);
        print_device_info(fmt1);

        if (info.field>0)
        {

            fmt.fmt.pix.field = info.field;
            fmt.fmt.pix.width = fmt1.fmt.pix.width;
            fmt.fmt.pix.height = fmt1.fmt.pix.height;
            fmt.fmt.pix.pixelformat = fmt1.fmt.pix.pixelformat;
            ioctl(info.fd, VIDIOC_S_FMT, &fmt);
            printf("Values updated to:\n");
            sleep(1);
            ioctl(info.fd, VIDIOC_G_FMT, &fmt1);
            print_device_info(fmt1);
        }

        if (info.hw_reset == TRUE)
        {
            int result = ioctl(info.fd, TW6869_HW_RESET_IOCTL);
            printf("hw_reset call result was: %d\n",result);
        }

        if (info.hw_reset_delay_ms > 0)
        {
            unsigned long hw_reset_delay = info.hw_reset_delay_ms * HZ /1000;
            if (hw_reset_delay >0)
            {
                printf("hw_reset_delay [%u ms] being send to driver, sending [%lu jiffies].\n", info.hw_reset_delay_ms, hw_reset_delay);
                int result = ioctl(info.fd, TW6869_HW_RESET_SET_DELAY_IOCTL, (void *)&hw_reset_delay);
                printf("hw_reset call result was: %d\n",result);
            }
            else
            {
                printf("hw_reset_delay [%u ms] invalid, results in value [%lu jiffies]. Must use a larger value.\n", info.hw_reset_delay_ms, hw_reset_delay);
            }
        }

        close_device();

}
