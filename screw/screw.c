#include "zencode.h"
#include <sys/stat.h>
#define PM9SCREW        "\tPM9SCREW\t"
#define PM9SCREW_LEN     10

short pm9screw_mycryptkey[] = {
	11152, 368, 192, 1281, 62
};

int decode_screw(char* filename)
{
	FILE *fp;
	char buf[PM9SCREW_LEN + 1];
	struct stat stat_buf;
	char *datap, *newdatap;
	int datalen, newdatalen;
	int cryptkey_len = sizeof pm9screw_mycryptkey / 2;
	char decode_filename[256];
	int i;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "File not found(%s)\n", filename);
		return 0;
	}

	fread(buf, PM9SCREW_LEN, 1, fp);
	if (memcmp(buf, PM9SCREW, PM9SCREW_LEN) != 0) {
		fprintf(stderr, "Not a crypted file.\n");
		fclose(fp);
		return 0;
	}

#ifdef WIN32
	fstat(_fileno(fp), &stat_buf);
#else
	fstat(fileno(fp), &stat_buf);
#endif

	datalen = stat_buf.st_size - PM9SCREW_LEN;
	datap = (char*)malloc(datalen);
	memset(datap, 0, datalen);
	fread(datap, datalen, 1, fp);
	fclose(fp);
	for (i = 0; i < datalen; i++) {
		datap[i] = (char)pm9screw_mycryptkey[(datalen - i) % cryptkey_len] ^ (~(datap[i]));
	}

	newdatap = zdecode(datap, datalen, &newdatalen);
	if (newdatalen)
	{
		sprintf(decode_filename, "%s%s", filename, ".decode");
		fp = fopen(decode_filename, "w");
		fwrite(newdatap, newdatalen, 1, fp);
		fclose(fp);
		fprintf(stderr, "Success Decrypt(%s)\n", filename);
		return 1;
	}
	return 0;
}

int encode_screw(char* filename) {
	FILE	*fp;
	struct	stat	stat_buf;
	char	*datap, *newdatap;
	int	datalen, newdatalen;
	int	cryptkey_len = sizeof pm9screw_mycryptkey / 2;
	char	oldfilename[256];
	int	i;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "File not found(%s)\n", filename);
		return 0;
	}

#ifdef WIN32
	fstat(_fileno(fp), &stat_buf);
#else
	fstat(fileno(fp), &stat_buf);
#endif
	datalen = stat_buf.st_size;
	datap = (char*)malloc(datalen + PM9SCREW_LEN);
	fread(datap, datalen, 1, fp);
	fclose(fp);

	sprintf(oldfilename, "%s.screw", filename);

	if (memcmp(datap, PM9SCREW, PM9SCREW_LEN) == 0) {
		fprintf(stderr, "Already Crypted(%s)\n", filename);
		return 0;
	}

	fp = fopen(oldfilename, "w");
	if (fp == NULL) {
		fprintf(stderr, "Can not create backup file(%s)\n", oldfilename);
		return 0;
	}
	fwrite(datap, datalen, 1, fp);
	fclose(fp);

	newdatap = zencode(datap, datalen, &newdatalen);

	for (i = 0; i < newdatalen; i++) {
		newdatap[i] = (char)pm9screw_mycryptkey[(newdatalen - i) % cryptkey_len] ^ (~(newdatap[i]));
	}

	fp = fopen(filename, "w");
	if (fp == NULL) {
		fprintf(stderr, "Can not create crypt file(%s)\n", oldfilename);
		return 0;
	}
	fwrite(PM9SCREW, PM9SCREW_LEN, 1, fp);
	fwrite(newdatap, newdatalen, 1, fp);
	fclose(fp);
	fprintf(stderr, "Success Crypting(%s)\n", filename);
	free(newdatap);
	free(datap);
	return(1);
}

main(int argc, char**argv)
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s e/d filename.\n", argv[0]);
		exit(0);
	}
	if (!strcmp(argv[1], "d"))
	{
		decode_screw(argv[2]);
	}
	else {
		encode_screw(argv[2]);
	}
}