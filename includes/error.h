#ifndef	ERROR_H
# define ERROR_H

# include <stdarg.h>
# include <errno.h>
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <netdb.h>

# define ERR_GAI_BASE	-10000
# define MAKE_GAI_ERR(rc)	( ERR_GAI_BASE + (rc) )
# define IS_GAI_ERR(rc)		((rc) <= ERR_GAI_BASE)
# define GET_GAI_CODE(rc)	((rc) - ERR_GAI_BASE)

static inline	int	ping_fail_errno(int e) { errno = e; return -1; }
static inline	int	ping_fail_gai(int rc) { return MAKE_GAI_ERR(rc); }

static inline	void	exit_error(int status, int errnum, char *format, ...) {
	va_list	ap;

	fprintf(stderr, "ft_ping: ");

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);

	if (errnum) {
		fprintf(stderr, ": %s\n", strerror(errnum));
	}
	else
		fprintf(stderr, "\n");

	if (status)
		exit(status);
}

#endif
