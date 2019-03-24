# StegIMG

This application allows to hide arbitrary data into bitmaps by simple steganography algorithm.  
The main idea of the algorithm is based on using lower bits of some components in RGBA to store of hidden data.  
`[A][R][G][B]` -- pixel presented in RGBA color space.  
In this scheme each byte D of input data is decomposed into four part, two bits each.  
Each part written to lower bits of R and/or B bytes.  
Here we use R and B parts only, because it allows to doesn't much spoil input image.  
