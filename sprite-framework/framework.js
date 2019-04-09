"use strict";

//Simple Sprite Framework namespace
function requireSSF()
{
    var ssf = {};

    //Namespaces
    ssf.Animator = makeAnimator();

    //Variables
    ssf.selectedSprite = SelectedSprite();
    ssf.global = Global();

    ssf.directions = {
        right: 0,
        left: 1,
        up: 2,
        down: 3
    };

    function Global()
    {
        var global = {};

        global.animators = new Map();

        global.addAnimator = function(animator)
        {
            global.animators.set(animator.name, animator);

            for(var i = 0; i < animator.events.length; i++)
                document.body.addEventListener(animator.events[i].type, animator.events[i].handler);

            return;
        };

        global.getAnimator = function(animator)
        {
            return global.animators.get(animator);
        };

        return global;
    }

    function SelectedSprite()
    {
        var selectedSprite = {};

        selectedSprite.this = null;
        selectedSprite.animators = new Map();

        selectedSprite.addAnimator = function(animator)
        {
            selectedSprite.animators.set(animator.name, animator);

            for(var i = 0; i < animator.events.length; i++)
                document.body.addEventListener(animator.events[i].type, forwarder.bind(animator));

            return;
        };

        function forwarder(event)
        {
            if(ssf.selectedSprite.this === null) return;
            else this.sprite = ssf.selectedSprite.this;

            for(var i = 0; i < this.events.length; i++)
            {
                if(this.events[i].type === event.type)
                    this.events[i].handler(event);
            }

            return;
        };

        selectedSprite.selector = function()
        {
            console.log("clicked");
            selectedSprite.this = this;

            return;
        };

        return selectedSprite;
    };

    //Background class
    ssf.Background = function(param)
    {
        var background = document.getElementById(param.id);

        if(background === null)
        {
            background = document.createElement("DIV");
            (param.parent || document.body).appendChild(background);
        }

        //Public variables
        background.id = param.id;
        background.className = param.className || "background";
        background.height = param.height || param.width || 500;
        background.width = param.width || param.height || 500;
        background.floor = param.floor || param.height || param.width || 500;
        background.image = param.image;
        background.parent = param.parent;

        background.style.height = background.height + "px";
        background.style.width = background.width + "px";

        if(background.image !== undefined)
            background.style.backgroundImage = background.image;

        background.makeSprite = function(param)
        {
            param.background = this;

            var ret = ssf.Sprite(param);

            this.appendChild(ret);

            return ret;
        };

        background.dropOnFloor = function(object)
        {
            object.style.top = this.floor - parseInt(object.style.height) + "px";

            return;
        };

        background.pushToRight = function(object)
        {
            object.style.left = this.width - parseInt(object.style.width) + "px";

            return;
        };

        background.pushToLeft = function(object)
        {
            object.style.left = "0px";

            return;
        };

        return background;
    };

    //Sprite class
    ssf.Sprite = function(param)
    {
        var sprite = document.getElementById(param.id);

        if(sprite === null)
            sprite = document.createElement("DIV");

        //Public variables
        sprite.index = 0;
        sprite.id = param.id;
        sprite.background = param.background;
        sprite.idle = param.idle;
        sprite.direction = ssf.directions.right;
        sprite.animations = new Map();
        sprite.animators = new Map();

        //Public functions
        sprite.setAnimation = function(animation)
        {
            if(animation === undefined) return;

            this.style.height = animation.height + "px";
            this.style.width = animation.width + "px";
            this.style.backgroundImage = animation.file;
            this.index = 0;

            return;
        };

        sprite.addAnimation = function(animation)
        {
            sprite.animations.set(animation.name, animation);

            return;
        };

        sprite.getAnimation = function(name)
        {
            return sprite.animations.get(name);
        };

        sprite.addAnimator = function(animator)
        {
            sprite.animators.set(animator.name, animator);
            animator.sprite = this;

            for(var i = 0; i < animator.events.length; i++)
                this.addEventListener(animator.events[i].type, animator.events[i].handler);

            return;
        };

        sprite.getAnimator = function(animator)
        {
            return sprite.animators.get(animator.name, animator);
        };

        sprite.setTop = function(top)
        {
            if(top > this.background.floor - parseInt(this.style.height))
            {
                this.style.top = this.background.floor - parseInt(this.style.height) + "px";
                return false;
            }
            else if(top < 0)
            {
                this.style.top = "0px";
                return false;
            }
            else
                this.style.top = top + "px";

            return true;
        };

        sprite.setLeft = function(left)
        {
            if(left > this.background.width - parseInt(this.style.width))
            {
                this.style.left = this.background.width - parseInt(this.style.width) + "px";
                return false;
            }
            else if(left < 0)
            {
                this.style.left = "0px";
                return false;
            }
            else
                this.style.left = left + "px";

            return true;
        };

        sprite.setDirection = function(direction)
        {
            this.direction = direction;
            this.style.backgroundPosition = this.idle.pose[direction][0];

            return;
        };

        //Constructor
        if(sprite.idle !== undefined)
        {
            sprite.setAnimation(sprite.idle);

            if(sprite.idle.pose !== undefined)
                sprite.style.backgroundPosition = sprite.idle.pose[sprite.direction][0];
        }
        else
        {
            sprite.style.width = window.getComputedStyle(sprite).getPropertyValue("width");
            sprite.style.height = window.getComputedStyle(sprite).getPropertyValue("height");
        }

        sprite.style.zIndex = param.zIndex || 1;
        sprite.setTop(0);
        sprite.setLeft(0);

        sprite.addEventListener("click", ssf.selectedSprite.selector);

        return sprite;
    };

    ssf.genRegPoses = function(param)
    {
        var pose = [];

        param.initialX = param.initialX || 0;
        param.initialY = param.initialY || 0;
        param.shiftX = param.shiftX || 0;
        param.shiftY = param.shiftY || 0;
        param.length = param.length || 0;

        for(var i = 0; i < param.length; i++)
            pose[i] = (param.initialX + param.shiftX * i)+"px "+(param.initialY + param.shiftY * i)+"px";

        return pose;
    };

    function makeAnimator()
    {
        var animator = {};

        animator.OnClick = function(param)
        {
            var onClick = {};

            var direction = param.direction || ssf.directions.right;
            var shift = param.shift || 20;

            onClick.events = [{type:"click", handler:clickHandler}];
            onClick.sprite = param.sprite;
            onClick.animationName = param.animationName || "move";
            onClick.name = param.name || "default";

            function clickHandler()
            {
                var sprite = onClick.sprite;
                var animation = onClick.sprite.getAnimation(onClick.animationName);

                if(sprite.index !== 0) return;

                if(animation !== undefined)
                {
                    sprite.setAnimation(animation);
                    animate(sprite, animation);
                }
                else move(sprite);

                return;
            };

            function animate(sprite, animation)
            {
                if(sprite.index < animation.pose[direction].length)
                {
                    sprite.style.backgroundPosition = animation.pose[direction][sprite.index++];

                    move(sprite);

                    setTimeout(animate, 100, sprite, animation);
                }
                else
                {
                    sprite.setAnimation(sprite.idle);
                    sprite.style.backgroundPosition = sprite.idle.pose[direction][0];
                }

                return;
            };

            function move(sprite)
            {
                if(direction === ssf.directions.left || direction === ssf.directions.right)
                    sprite.setLeft(parseInt(sprite.style.left) + shift);
                else if(direction === ssf.directions.up || direction === ssf.directions.down)
                    sprite.setTop(parseInt(sprite.style.top) + shift);

                return;
            }

            if(direction === ssf.directions.left) shift *= -1;
            else if(direction === ssf.directions.up) shift *= -1;

            return onClick;
        };

        animator.KeyPad = function(param)
        {
            var keyPad = {};

            var shift = param.shift;
            var keyTable = param.keyTable;
            var waitTime = param.waitTime;
            var interval = null;

            keyPad.events = [{type:"keydown", handler:keyDownHandler}, {type:"keyup", handler:keyUpHandler}];
            keyPad.sprite = param.sprite;
            keyPad.animationName = param.animatonName || "move";
            keyPad.name = param.name || "default";

            function keyDownHandler(event)
            {
                var sprite = keyPad.sprite;
                var animation = sprite.getAnimation(keyPad.animationName);

                if(interval !== null || sprite === null) return;

                for(var i = 0; i < keyTable.length; i++)
                {
                    if(event.keyCode === keyTable[i].keyCode)
                    {
                        if(keyTable[i].direction !== sprite.direction)
                            sprite.direction = keyTable[i].direction;

                        if(animation !== undefined && animation.pose[sprite.direction] !== undefined)
                        {
                            sprite.setAnimation(animation);

                            animate(sprite, animation);
                            interval = setInterval(animate, waitTime, sprite, animation);
                        }
                        else move(sprite);
                    }
                }

                return;
            }

            function keyUpHandler(event)
            {
                var sprite = keyPad.sprite;

                if(sprite === null) return;

                for(var i = 0; i < keyTable.length; i++)
                {
                    if(event.keyCode === keyTable[i].keyCode)
                    {
                        if(sprite.idle !== undefined)
                        {
                            if(interval !== null) clearInterval(interval);

                            sprite.setAnimation(sprite.idle);
                            sprite.style.backgroundPosition = sprite.idle.pose[sprite.direction][0];

                            interval = null;
                        }

                        break;
                    }
                }

                return;
            }

            function animate(sprite, animation)
            {
                move(sprite);

                sprite.style.backgroundPosition = animation.pose[sprite.direction][sprite.index];

                if(++sprite.index >= animation.pose[sprite.direction].length)
                    sprite.index = 0;

                return;
            }

            function move(sprite)
            {
                var curShift = shift * (sprite.direction === ssf.directions.left || sprite.direction === ssf.directions.up ? -1 : 1);

                if(sprite.direction === ssf.directions.left || sprite.direction === ssf.directions.right)
                    sprite.setLeft(parseInt(sprite.style.left) + curShift);
                else if(sprite.direction === ssf.directions.up || sprite.direction === ssf.directions.down)
                    sprite.setTop(parseInt(sprite.style.top) + curShift);

                return;
            }

            return keyPad;
        };

        return animator;
    }

    return ssf;
}
