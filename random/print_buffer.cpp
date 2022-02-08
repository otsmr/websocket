

// std::fstream f;
// f.open("buffer.txt", std::ios::app);
// int j;
// for (size_t i = 0; i < bytes_read; i++)
// {
//     if (i < (bytes_read-17)) {
//         for (j = i; j < (i+17); j++) {
//             if (buffer[j] != '\00')
//                 break;
//         }
//         if (j > i+15) {
//             f << " [ 0x00 * " << (bytes_read) - i << " ]\n";
//             break;
//         }
//     }
//     char hex[4];
//     snprintf(hex, 4, "%02x", buffer[i]);
//     f << hex[0] << hex[1];
//     if ((i+1) % 4 == 0)
//         f << " ";
//     if ((i+1) % (4*8) == 0)
//         f << "\n";
// }
// f << "\n----------------------------------------------------- \n";
// f.close();