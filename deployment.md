# Copying Blender Plugin
There is a folder with the name 'sortblend' in SORT root folder, which has all blender plugin python source code in it. In order to add SORT in Blender as a new renderer, it is mandatory to copy the source code folder to the following folder,
* **Windows**  
C:\Program Files\Blender\2.79\scripts\addons
* **Mac OS**  
~/Library/Application Support/Blender/2.79/script/addons/
* **Ubuntu**  
/home/$user/blender/2.79/scripts/addons/

Depending the exact version of Blender, the above paths could be slightly different. However, the paths should be very similar with them.

# Activate SORT in Blender
After compiling SORT with source code, there should be a binary folder with the name 'bin' in SORT root folder. Usually, the binary is named 'sort_r' for release version build and 'sort_d' for debugging version build. It is also fine to generate or copy the binary to any other desired folder with other binary file names.

Open the 'Blender User Preferences' in Blender by choosing 'User Preferences' in 'File' menu.
![](https://lh3.googleusercontent.com/sNuHRHocQMSzZuZ6tciewPvTIDvoiu2CMMTkSipb_SISA9CE3lo5Z9s3E8zXnObX9K6aOvrRr1ZWlgyvFYYHLh7WdswgNWXZgPeJc73fxavc3Ap4rRyLTTrCwOKJCPb7QjcwO0vQpviCX0B2HR594gxFXvb3E2TAryRRc6-BEneDwmJcZFmP6wDaDStpS-xBQcrKq79YS86u8bM3Met9zwXkqtkOLsB0j4STUdB51Zwi-rTfdUu7hMhRY_QpptC-4AhiB9suXLWK5e8wL6Wwirzv8ByR1ryFawF-i41_sBZ2H4wZupML-u-U6NjcRzP1Ct7rb6OBI6Od9ZhrZFkYVmQJdENv-9qpEwyWKUg5p5NC5oSHpTJb7d3MV6alQmplx4ECEVTVEPOLnnUNcDGmktfWK3pIJqh8dbUlht6VAIKmw1qbm4EkFlpZ3rZ5D1HMlWJqN0qst8cSSRtAWDNO8SZHujZoqSYjxaA8p_IkIc0AwEFiX6Utt2eiWEF_TMxVukx1a6tiUPqhUnvkzqfN7mKBKbf5KIlfZBt-1ovwpXvp5_ECnSf9AZgkps2PQayhp0Fvx3qEeaMk47qM3fBBquNlUvfIdxEKlCJK7y0sqLdB19CKjjvLkVcCjgHQwMU5UnlNrM2PRuT4OSMX4wJIt81yhKulzjuv=w2302-h1352-no)
Tick the checkbox before SORT and set the path of SORT binary to the folder that has SORT binary.
There is another optional path to be set, it is supposed to be set to the binary path of PBRT so that it can export some scene to PBRT for a reference rendering result. The compilation of PBRT is out of scope of this wiki, please refer [this page](https://pbrt.org/) for further detail. This path is totally optional, meaning SORT will work just fine without it being set to anything.

# Picking SORT as the Renderer
After SORT is activated in Blender, the next step is to pick SORT as the renderer in Blender so that Blender will send jobs to SORT for rendering.
![](https://lh3.googleusercontent.com/MsBSI2KuxyLsBGBF3wxVsZcujYcd7IpziIdOVHZgm6DrG2_fgUwCop_qmiytIJn8ndZopCiyTOmKYRsW2ooPd0XcyyO4QUA64u-MnpUzgFulAcTwLJpyYW8PS-YnW6ge1IdChmeHCJdKxFndTmZJfYurBWzn7wZDHa6kQICzQDbQtQP7a4_P6jZBL5JAz_0wIYLb62GsHHbF83wcQZ8an68Q8KgzgJpXqgrzuYlhfj_0Bg8kb2r_SNhp_Q97bh0EQiywX0O-QQQVvv-uk5u0elx8oqKwi8dkqHCA8PhiDilMdThUvozM97WJFcS3_tASxpJoNsrHGX-Av2z0RnJ7tY1msAlAH6dUQrS4mBlY70TcYtMsAj9HoAJWhvVXVtd0DExDcS83ulKdFUEkvxTyshXXTQhmq2i0kwgTktdeADKQOOZumQBOXsfAwUuuBJ4d2Ttqt9eOOFG1ydBqrM47alZs02-8tbXI1SFn4eko9F-pkKpV1Y6GgXBu1CBCAlo9OXsgFrwAczI1XzP5d9k_1kBjQh3K3qYJ31qX21pyNXiYH_dvY0x9ei6MrrcFeUhpyXdXeoGcnWpG2BijGbPtqL4ju-FTqpv9GOBQ2a0rHMgus-dhRbeHF8gaiJbWZf4kEtRdGffzWZRgmMgkAP1paEwGOQZbSCZb=w2302-h1346-no)
Simply by choosing SORT in engine will pick it as the main renderer. The rendering process is no difference from other engines.