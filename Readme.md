# Через тернии к фаззингу
Чтобы увеличить эффективность фаззинга, я написал кастомную мутацию для файлов формата png.

### Простейший png файл имеет следующую структуру:

![1](/images/1.png)

[Источник изображения](https://habr.com/ru/articles/130472/)


Файл состоит из чанков - блоков с такой структурой

![1](/images/2.png)

[Источник изображения](https://habr.com/ru/articles/130472/)

Длина - целое число, хранящее длину блока данных в байтах

Тип - IDAT/IHDR/...

Данные - понятно

CRC - хеш , высчитывающийся для полей тип и данные

Для высчитывания CRC я взял ее реализацию с [сайта спецификаций png](http://www.libpng.org/pub/png/spec/1.2/PNG-CRCAppendix.html)

## PNG signature
В начале каждого png файла должна быть такая подпись:

`89 50 4E 47 0D 0A 1A 0A`

Поэтому в начале генерации мы должны записать подпись:
```C
unsigned char signature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
fwrite(signature, sizeof(char), 8, picture);
```
## IHDR
Этот чанк должен стоять самым первым, так как он содержит основную информацию об изображении: высота, ширина, глубина, тип цвета... . 

Пример генерации:

```C
    chunk IHDR = {};
    IHDR.length = IHDR_LENGTH;     
    strcpy(IHDR.type, "IHDR");  

    IHDR.data = (char *) calloc(IHDR_LENGTH, sizeof(char));

    IHDR.data[3] = (char) WIDTH;            
    ...

    IHDR.data[7] = (char) HEIGHT;
    ...

    IHDR.data[8] = 1;   // bit depth
    IHDR.data[9] = 0;   // colour type (greyscale)

    IHDR.data[10] = 0;  // weave method        (const)
    IHDR.data[11] = 0;  // compression method  (const)
    IHDR.data[12] = 0;  // filtration          (const)
    write_chunk(file, &IHDR);
```

## IDAT
Данный чанк содержит основную информацию об изображении.

Пример генерации:

```C
chunk IDAT = {};
strcpy(IDAT.type, "IDAT");

IDAT.length = WIDTH * HEIGHT * 3;

IDAT.data = (char *)calloc(IDAT.length + 1, sizeof(char));
for (int i = 0; i < IDAT.length; i++) {
    IDAT.data[i] = rand() % 256;
}

write_chunk(file, &IDAT);
```

## IEND
Завершающий чанк, который не несет никакой информации


Пример генерации:
```C
chunk IEND = {};
strcpy(IEND.type, "IEND");
IEND.length = 0;
write_chunk(file, &IEND);
```

# Еще чето 

# Источники

https://iter.ca/post/png/

https://habr.com/ru/articles/130472/

http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html

https://habr.com/ru/companies/ruvds/articles/787302/

http://www.libpng.org/pub/png/spec/1.2/PNG-CRCAppendix.html
