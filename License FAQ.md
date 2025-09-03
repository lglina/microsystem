# License FAQ
I've chosen to license the Micro System hardware and software source files under
a "source available" license, the [Polyform Shield License 1.0.0.](https://polyformproject.org/licenses/shield/1.0.0/),
rather than an open source/free software license. This is a plain-language
explainer for why I chose to do that and how I feel about software licensing
in general, but should not be seen as a substitute for, or adding to or deleting
from, the license text in any way, or as permission for you to do a particular
thing. Please read the license and seek your own legal advice if required.

## What am I allowed to do with the code?
In creating a system that is intended for hacking, learning and experimentation,
I want you to be able to:

* See all the source code
* Build it from source and run it on your own hardware
* Modify the source code and share those modifications with others

The license allows you to do all these things, except...

## What am I not allowed to do with the code?
You're not allowed to use the software to provide a product that competes
with me, for example, producing clones of the Micro System 1 kit and selling
them pre-loaded with the software.

Please read further to understand why I feel this is a necessary condition, to
protect the community from freeloaders who might seek to create poor quality and
unsafe products with the software while giving nothing back.

## How is this different from a free software or open source license?
The essay [What is Free Software](https://www.gnu.org/philosophy/free-sw.en.html)
says that free software provides users with four essential freedoms: the freedom
to run the program as you wish, the freedom to study how the program works and
change it, the freedom to redistribute copies, and the freedom to distribute
copies of modified versions.

"Source available" licenses differ in that they don't give you "freedom zero" -
the freedom to run the program as you wish, for any purpose. In the case of the
Polyform Shield license, you are not permitted a license to the software if
your purpose is to compete with the software or any product I provide using
the software.

Arguably "source available" licenses also do not satisfy points five and six of
the [Open Source Definition](https://opensource.org/osd), in that they
discriminate against particular users or fields of endeavour.

## OK, that's a lot of jargon! What does this practically mean for me?
For most people, probably nothing. If you buy one of my computer kits, I really
want you to be able to see the source code, hack it, and freely share your
changes with others. In the spirit of hacker culture, I'm sharing this cool
thing I made and I want to create a community of people around it who will help
to make it better.

## So why not just use a free software/open source license?
Free software/open source licenses were once an effective tool to counter
closed source/proprietary licenses and the companies who used them, and to
give freedom back to the community of hackers and computer users. I believe,
however, that the use of those same licenses now risks harming the communities
that they were supposed to be helping, and that there is a failure by the
proponents of those licenses to recognise and address those harms. (Quite the
opposite - those proponents view "source available" licenses as doing harm.)

What does this mean in the context of this software?

I've put a lot of work into creating a computer kit and associated software that
I hope people will find enjoyable and educational, and if I'm rewarded for that
effort (by way of people buying the kit, supporting me on YouTube, etc.) then
I can give back to users of the software, and the broader community, by making
the software and hardware even better, and creating new, awesome things.

What I don't want to see, though, are people using my software (or creating
hardware that uses my software) purely to make money for themselves with no
intention of making the hardware or software better or giving anything back to
the community, or, worse, using my software and/or hardware designs to harm the
community. Think low-quality, unsafe kits; or the Micro System software being
hosted online by companies who charge for access but give nothing back, or 
monetise the software for their own purposes via advertising or stealing
user data.

A "source available" license gives me a legal footing to stop those abusive
use cases, while (in my opinion), not unduly restricting the freedoms of people
who just want to see, hack and share the software.

A free software/open source license, in contrast, requires that the software
author allow anyone to do anything with the software regardless of how
damaging that use might be to the author, the community creating and supporting
the software, or the broader community. I believe it's time to have a real
discussion around the reasonable use of (minimal) license restrictions to
avoid harm, rather than doggedly holding to the position (that I feel is
extreme) that any restrictions are unacceptable.

Unfortunately, in a similar way to how [free software/open source licenses have failed to protect database projects from freeloading cloud providers](https://www.infoworld.com/article/2335703/open-source-needs-to-catch-up-in-2024.html), these
licenses also fail to protect my kind of "software-driven-hardware" project
from [freeloading hardware cloners](https://www.youtube.com/watch?v=IWIROMu5l6w).

The alternative is open-core ([a failing licensing model?](https://joemorrison.medium.com/death-of-an-open-source-business-model-62bc227a7e9b)), or closed-source, neither of which help you to understand how the software works, allow you to modify it, or repair or exercise your ownership
rights over hardware running it.

The problems of freeloading and abusive cloning have been discussed for a while,
but there now does seem to be a broader conversation around the parallel
issue of reputational damage by way of low quality or malicious cloning. See
[FUTO's The Open Source Definition](https://futo.org/open-source-definition/).

## Aren't you just trying to enrich yourself/be monopolistic with a non-compete? Cheaper competing products mean more people can use your devices/software, and that's better for the community!
Quality and safety are really important when it comes to hardware products,
especially those designed for use by children. [There's a huge difference, for example, between well-engineered switch-mode power supplies and poorly designed ones](https://www.righto.com/2015/11/). While
both look identical, things like proper primary and secondary separation, the
use of triple insulated wire, and attention to detail in transformer winding,
are the difference between a safe, reliable power supply and one that can
quickly become deadly to the touch.

I no longer buy no-name chargers/power supplies and I would never let my
children use them. For that reason, I refuse to ship a poor quality power supply
with the Micro System 1 kit, even if it means I make less money from each kit.
I will only supply quality "MEAN WELL" power supplies purchased from authorised
stockists and with all required electrical safety approvals.

I feel there is a role for licensing to play in preventing people simply
making cheap and dangerous copies of my hardware, then putting that out into
the world running my software.

It's important to note that none of this prevents anyone making competing
hardware and software along the same lines as anything I've done. You just
can't sell an exact copy or something substantially based on my code. Make
something that's uniquely yours, or talk to me about alternative licensing
(see below).

Also, nothing prevents you from making your own kit from my hardware designs,
for your own personal use, and using whatever quality or brand of parts you
like. Or, repairing or modifying anything you buy from me in whatever way you
want. You do you. Just don't sell low quality clones of my stuff.

## I'm still not clear - are you going to sue me?
Without wanting to go into every possible thing you might do with the software,
these are some of the things I'd think are OK vs. not OK:

OK:
* I want to compile and run the software on my own computer
* I want to make copies of the software for my friends/install it on their
  computers
* I want to hack the software or add new features and share those changes with
  other people via my website or by forking on Github
* I want to make my own Micro System 1 computer kit for my own personal use,
  using your hardware design files, then run the software on it
* I want to make a cool new peripheral for the Micro System 1 computer kit and
  do a Kickstarter or sell it on my website

Not OK (without permission):
* I want to make clones of the Micro System 1 kit and sell them on my website or
  in my shop
* I want to host the software as a service on my website or in the cloud
* I want to make a clone of some peripheral that Lauren is already selling and
  sell it myself

Please note that there may also be issues if you want to want to sell something
and you use any of my trademarks.

## I hate this license and you're a terrible human
OK, that's not a question, but it should be pointed out that there's nothing
stopping me from granting a separate license (and it could even be a royalty-
free license!) to anyone who wants to provide some sort of product and is
worried about the non-compete clause. Please just get in touch and we can talk
about it!

Hopefully you can appreciate that it's not about preventing people doing
things in good faith, but preventing bad faith commercial exploitation of my
work by people who don't care about quality, safety, and you, the customer
or user.

I'm no lawyer, but I'm always happy to have a discussion around whether this is
the right license for this project and, who knows, perhaps I can eventually
have a real software lawyer write me something better. Please reach out if
you want to chat about it, but be warned that I probably won't write back if
your email starts with "duh, but freedom 0, and field of use restrictions
are BAD...".

Much love,

Lauren Glina
29 August 2025
