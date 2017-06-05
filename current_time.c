#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/time.h>

#define MODULE_NAME "current_time"

static struct task_struct *task;

static void current_time_loop(void)
{
	struct file *file;
	const char *fname = "/tmp/current_time";

	struct timespec ts;
	struct tm tm;

	char buf[7];  /* with space for NULL */
	const char *fmt = "%02d:%02d\n";

	/* 1 minute. */
	const unsigned long timeout = msecs_to_jiffies(60000);

	int ret;

	file = filp_open(fname, O_CREAT | O_WRONLY, 0644);
	if (IS_ERR(file)) {
		ret = PTR_ERR(file);
		pr_err(MODULE_NAME ": failed to open '%s': %d\n", fname, ret);
		goto out;
	}

	while (!kthread_should_stop()) {
		getnstimeofday(&ts);
		time_to_tm(ts.tv_sec, 0, &tm);  /* no offset seconds */

		/* Will NULL-terminate. */
		ret = snprintf(buf, sizeof(buf), fmt, tm.tm_hour, tm.tm_min);
		/* Doesn't count the NULL byte. */
		if (ret != sizeof(buf) - 1) {
			pr_err(MODULE_NAME
			       ": failed writing to buffer; bytes written: %d\n",
			       ret);
			goto out_close;
		}

		/* Do not write NULL. */
		ret = kernel_write(file, buf, sizeof(buf) - 1, 0);
		if (ret < 0) {
			pr_err(MODULE_NAME ": failed writing to file '%s': %d\n",
			       fname, ret);
			goto out_close;
		}

		/* Do not wait for 'timeout' on 'kthread_stop'. */
		set_current_state(TASK_INTERRUPTIBLE);
		if (kthread_should_stop()) {
			goto out_close;
		}
		schedule_timeout(timeout);
	}

out_close:
	ret = filp_close(file, NULL);
	if (ret < 0) {
		pr_err(MODULE_NAME ": failed to close '%s': %d\n", fname, ret);
	}

out:
	task = NULL;
}

static int __init current_time_init(void)
{
	const char *tname = "current_time_loop";
	int ret = 0;

	pr_info(MODULE_NAME ": loaded\n");

	task = kthread_run((void *)current_time_loop, NULL, tname);
	if (IS_ERR(task)) {
		ret = PTR_ERR(task);
		pr_err(MODULE_NAME ": failed to start thread '%s': %d\n",
		       tname, ret);
		goto out;
	}

out:
	return ret;
}

static void __exit current_time_exit(void)
{
	if (task) {
		kthread_stop(task);
	}

	pr_info(MODULE_NAME ": unloaded\n");
}

module_init(current_time_init);
module_exit(current_time_exit);

MODULE_DESCRIPTION("Save current time to a file every minute");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nikita Karetnikov <nikita@karetnikov.org>");
