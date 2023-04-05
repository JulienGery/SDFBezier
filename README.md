# Branch for testing

<!-- Based on [Viktor Chlumský](https://github.com/Chlumsky/msdfgen) Master's thesis.   -->
<!-- Use [Walnut](https://github.com/TheCherno/Walnut). -->
## Building and running
You'll need to have the [Vulkan SDK](https://vulkan.lunarg.com/) installed.

1. Clone recursively: `git clone -b sandBox --recursive https://github.com/JulienGery/SDFBezier`
2. Run `scripts/Setup.bat`
3. Open `SDFBezier.sln` and hit F5 (preferably change configuration to Release or Dist first, Debug is slow)

the goal of this branch is to have a sanitize environement to debug shader.