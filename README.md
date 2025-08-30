# Vinkan
Vinkan is a personal Vulkan wrapper that cuts through repetitive setup code for developers who already know their way around Vulkan. It's not about making Vulkan "beginner-friendly"—it's about avoiding the tedious copy-paste ritual every time you start a new project.

## Purpose
Skip the ceremony: Turn ~1000 lines of setup into ~100 lines of actual work
Eliminate boilerplate: No more rewriting queue family queries, basic framebuffer setup, and memory allocation patterns for every project
Keep the important stuff: Full access to Vulkan objects when you need them
Faster prototyping: Less time on setup, more time building

## What It Actually Does
Vinkan handles the common patterns you end up writing every time—finding the right queue families for your specific needs, setting up basic framebuffers, and managing the repetitive initialization dance. It assumes you know what you want and just gets out of your way.

## Who This Is For
The first is me, but also people who,
- Understand render passes, command buffers, and synchronization
- Know when they need specific Vulkan features
- Can debug graphics issues without hand-holding
- Are tired of writing the same queue setup code for every project

Built for personal use where getting to the interesting parts quickly matters more than learning fundamentals from scratch.
