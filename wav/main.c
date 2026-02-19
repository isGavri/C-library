#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef int16_t i16;
typedef uint16_t u16;
typedef uint32_t u32;

typedef float f32;

void write_le_16(FILE* f, u16 n)
{
    fwrite(&n, sizeof(u16), 1, f);
}

void write_le_32(FILE* f, u32 n)
{
    fwrite(&n, sizeof(u32), 1, f);
}

#define WRITE_STR_LIT(f, s) fwrite((s), 1, sizeof(s) - 1, f)

#define FREQ 44100

int main(void)
{
    FILE* f = fopen("test.wav", "wb");

    // Notes with frequency and duration
    struct
    {
        f32 freq;
        f32 dur;
    } notes[] = {
        {392, 60.0f / 76}, {440, 60.0f / 76}, {294, 60.0f / 114},
        {440, 60.0f / 76}, {494, 60.0f / 76},
    };

    u32 num_notes = sizeof(notes) / sizeof(notes[0]);

    f32 duration = 0.0f;
    for (u32 i = 0; i < num_notes; i++)
    {
        duration += notes[i].dur;
    }

    // *** Header *** //
    u32 num_sample = (u32)(duration * FREQ);
    // Size of header plus samples
    u32 file_size = num_sample * sizeof(u16) + 44;

    // FileTypeBlocID - Identifier
    WRITE_STR_LIT(f, "RIFF");
    // FileSize - Overall file size minus 8 bytes
    write_le_32(f, file_size - 8);
    // FileFormatID - Format
    WRITE_STR_LIT(f, "WAVE");

    // *** Describes the data format *** ///
    // FormatBlocID - Identifier
    WRITE_STR_LIT(f, "fmt ");
    // BlocSize - 16bytes constant
    write_le_32(f, 16);
    // AudioFormat - PCM Integer
    write_le_16(f, 1);
    // Number of channels - 1
    write_le_16(f, 1);
    // Sample rate
    write_le_32(f, FREQ);
    // BytePerSec - FREQ * BytePerBloc
    write_le_32(f, FREQ * sizeof(u16));
    // BytePerBloc - Channels * BitsPerSample / 8
    write_le_16(f, sizeof(u16));
    // BitsPerSample
    write_le_16(f, sizeof(u16) * 8);

    // *** Chunk Containing the sampled data *** //
    // DataBlocID
    WRITE_STR_LIT(f, "data");
    // DataSize
    write_le_32(f, num_sample * sizeof(u16));


    // *** Writing Sample *** //

    u32 cur_note = 0;
    f32 cur_note_start = 0.0f;
    for (u32 i = 0; i < num_sample; i++)
    {
        f32 t = (f32)i / FREQ;
        f32 y = 0.0f;
        if (cur_note < num_notes)
        {
            y = 0.25f * sinf(t * notes[cur_note].freq * 2.0f * 3.1415926535f);

            if (t > cur_note_start + notes[cur_note].dur)
            {
                cur_note++;
                cur_note_start = t;
            }
        }

        i16 sample = (i16)(y * INT16_MAX);

        write_le_16(f, sample);
    }

    fclose(f);

    return 0;
}
