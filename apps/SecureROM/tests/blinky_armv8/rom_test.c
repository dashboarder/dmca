void start(void)
{
    *((volatile unsigned int *)GPIO_BASE) = GPIO_VALUE;
    for(;;);
}
