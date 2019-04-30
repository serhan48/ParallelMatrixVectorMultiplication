#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>


#define TRUE 1

// COMPLEX SAYI ICIN STRUCT TANIMLA

struct complex_number {
	double real_part;
	double imaginary_part;
};

typedef struct complex_number complex_num_t;


// THREADLERIN UZERINDE ISLEM YAPACAGI GLOBAL MATRIS VE VEKTORLER

complex_num_t* vector = NULL;
complex_num_t* matrix = NULL;
complex_num_t* result_vector = NULL;

int matrix_row_num, matrix_col_num;
int thread_num = 0;

// THREADLERIN HESAPLAYACAGI BIR SONRAKI SATIRI TUTAN DEGISKEN VE MUTEXI

int row_counter = 0;
pthread_mutex_t row_counter_mutex = PTHREAD_MUTEX_INITIALIZER;


// COMPLEX SAYILARIN CARPIMI ICIN FONKSIYON

complex_num_t multiply_complex(const complex_num_t c1, const complex_num_t c2)
{
	complex_num_t result;

	result.real_part = (c1.real_part * c2.real_part) - (c1.imaginary_part * c2.imaginary_part);
	result.imaginary_part = (c1.real_part * c2.imaginary_part) + (c1.imaginary_part * c2.real_part);

	return result; 
}

// COMPLEX SAYILARIN TOPLAMI ICIN FONKSIYON

complex_num_t add_complex(const complex_num_t c1, const complex_num_t c2)
{
	complex_num_t result;

	result.real_part = c1.real_part + c2.real_part;
	result.imaginary_part = c1.imaginary_part + c2.imaginary_part;

	return result;
}

// MATRIS SATIRININ VEKTOR ILE CARPIMI ICIN FONKSIYON

void* multiply_row(void* thread_arg)
{

	while (TRUE)
	{


		pthread_mutex_lock(&row_counter_mutex);
		int row = row_counter;
		row_counter++;

		pthread_mutex_unlock(&row_counter_mutex);

		if (row >= matrix_row_num)
			break;

		int i;

		complex_num_t result;
		result.real_part = 0.0;
		result.imaginary_part = 0.0;

		for (i = 0; i < matrix_col_num; i++)
		{
			result = add_complex(result, multiply_complex(matrix[row * matrix_col_num + i], vector[i]));
		}

		result_vector[row] = result;


	}

	pthread_exit(NULL);
	
	
}

// MATRIS DOSYASINDAKI VERILERI PARSE EDIP OKU

int read_from_file(FILE* file, complex_num_t* array)
{

	double real_part, imaginary_part;

	char str[200];
	str[0] = '\0';

	char c;

	int num_read = 0;


	while ((c = fgetc(file)) != EOF)
	{


		int i = 0;

		while (c != EOF && c != '\n' && c != ',')
		{
			str[i] = c;

			c = fgetc(file);
			i++;

		}

		str[i] = '\0';

		i = 0;

		while (str[i] != '\0')
		{
			if (str[i] == '+')
			{
				
				if (str[i-1] == 'e')
					continue;
				else
				{
					str[i] = '\0';
					sscanf(str, "%lf", &real_part);
					sscanf(&str[i + 1], "%lf", &imaginary_part);
					str[i] = '+';

					break;
				}
				
			}

			else if (str[i] == '-')
			{
				if (i == 0)
				{
					int j = 1;

					while (str[j] != '\0')
					{
						if (str[j] == '-')
						{
							if (str[j - 1] == 'e')
							{
								i++;
								continue;
							}
							else
							{
								str[j] = '\0';
								sscanf(str, "%lf", &real_part);
								sscanf(&str[j + 1], "%lf", &imaginary_part);
								str[j] = '-';
								imaginary_part = -imaginary_part;

								break;

							}
							
						}

						else if (str[j] == '+')
						{
							if (str[j - 1] == 'e')
							{
								i++;
								continue;
							}
							else
							{
								str[j] = '\0';
								sscanf(str, "%lf", &real_part);
								sscanf(&str[j + 1], "%lf", &imaginary_part);
								str[j] = '+';

								break;
							}
							
						}

						else if (str[j] == 'i')
						{
							sscanf(str, "%lf", &imaginary_part);
							real_part = 0.0;

							break;
						}

						j++;
					}

					if (str[j] == '\0')
					{
						sscanf(str, "%lf", &real_part);
						imaginary_part = 0.0;

					}

					break;
				}

				else
				{
					if (str[i - 1] == 'e')
					{
						i++;
						continue;
					}
					else
					{
						str[i] = '\0';
						sscanf(str, "%lf", &real_part);
						sscanf(&str[i + 1], "%lf", &imaginary_part);
						str[i] = '-';
						imaginary_part = -imaginary_part;

						break;
					}
					
				}
			}

			else if (str[i] == 'i')
			{
				sscanf(str, "%lf", &imaginary_part);
				real_part = 0.0;

				break;
			}

			i++;
		}

		if (str[i] == '\0')
		{
				sscanf(str, "%lf", &real_part);
				imaginary_part = 0.0;
		}



		array[num_read].real_part = real_part;
		array[num_read].imaginary_part = imaginary_part;

		num_read++;


	}

	return num_read;
}




int main()
{


	char matrix_file_name[100], vector_file_name[100];
	
	FILE* matrix_file;
	FILE* vector_file;

	pthread_t* threads;

	clock_t start, end;
	double time_spent;


	// MATRIS DOSYASI BILGILERINI OKU VE DOSYAYI AC

	printf("MATRIS BOYUTUNU GIRINIZ: ");
	scanf("%d %d", &matrix_row_num, &matrix_col_num);

	printf("LUTFEN MATRIS DOSYA ADINI GIRINIZ: ");
	scanf("%s", matrix_file_name);

	matrix_file = fopen(matrix_file_name, "r");

	if (matrix_file == NULL)
	{
		printf("HATA: %s MATRIS DOSYASI ACILAMADI!\n", matrix_file_name);
		exit(-1);
	}

	// DOSYADAN OKUNAN MATRIS VE VEKTORU BELLEKTE SAKLAMAK ICIN ALAN

	matrix = (complex_num_t *)malloc(matrix_row_num * matrix_col_num * sizeof(complex_num_t));

	if (matrix == NULL)
	{
		printf("HATA: YETERLI BELLEK ALANI YOK!");
		exit(-1);
	}

	if (read_from_file(matrix_file, matrix) < matrix_row_num * matrix_col_num)
	{
		printf("HATA: %s DOSYASINDAN TUM ELEMANLAR OKUNAMADI!\n", matrix_file_name);
		exit(-1);
	}
	

	// VEKTOR DOSYASI BILGILERINI OKU VE DOSYAYI AC

	printf("LUTFEN VEKTOR DOSYA ADINI GIRINIZ: ");
	scanf("%s", vector_file_name);

	vector_file = fopen(vector_file_name, "r");

	if (vector_file == NULL)
	{
		printf("HATA: %s VEKTOR DOSYASI ACILAMADI!\n", vector_file_name);
		exit(-1);
	}


	vector = (complex_num_t *)malloc(matrix_col_num * sizeof(complex_num_t));

	if (vector == NULL)
	{
		printf("HATA: YETERLI BELLEK ALANI YOK!");
		exit(-1);
	}

	result_vector = (complex_num_t *)malloc(matrix_row_num * sizeof(complex_num_t));

	if (result_vector == NULL)
	{
		printf("HATA: YETERLI BELLEK ALANI YOK!");
		exit(-1);
	}

	if (read_from_file(vector_file, vector) < matrix_col_num)
	{
		printf("HATA: %s DOSYASINDAN TUM ELEMANLAR OKUNAMADI!\n", vector_file_name);
		exit(-1);
	}



	// THREAD SAYISINI GIRDI OLARAK ALIP THREAD OLUSTUR

	printf("THREAD SAYISINI GIRINIZ: ");
	scanf("%d", &thread_num);

	threads = (pthread_t *)malloc(thread_num * sizeof(pthread_t));

	if (threads == NULL)
	{
		printf("HATA: YETERLI BELLEK ALANI YOK!");
		exit(-1);
	}


	start = clock();


	// THREADLERI OLUSTUR

	int i;
	for (i = 0; i < thread_num; i++)
	{
		if (pthread_create(&threads[i], NULL, multiply_row, NULL))
		{
			printf("HATA: THREAD OLUSTURULAMADI!");
			exit(-1);
		}
	}

	// THREADLERIN BITMESINI BEKLE

	for (i = 0; i < thread_num; i++)
	{
		pthread_join(threads[i], NULL);
	}

	end = clock();

	time_spent = (double)(end - start) / CLOCKS_PER_SEC;

	// SONUC VEKTORUNU EKRANA YAZDIR

	for (i = 0; i < matrix_row_num; i++)
	{
		if (result_vector[i].imaginary_part < 0)
			printf("[%d]:%lf%lfi\n", i, result_vector[i].real_part, result_vector[i].imaginary_part);
		else
			printf("[%d]:%lf+%lfi\n", i, result_vector[i].real_part, result_vector[i].imaginary_part);
	}

	printf("THREAD SAYISI:%d\n", thread_num);
	printf("SURE:%lf\n", time_spent);


	// KAYNAKLARI SERBEST BIRAK

	free(matrix);
	free(vector);
	free(result_vector);
	free(threads);

	fclose(matrix_file);
	fclose(vector_file);



	return 0;
}