#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>

MODULE_AUTHOR("DevTITANS <devtitans@icomp.ufam.edu.br>");
MODULE_DESCRIPTION("Driver de acesso ao SmartLamp (ESP32 com Chip Serial CP2102)");
MODULE_LICENSE("GPL");


#define MAX_RECV_LINE 100 // Tamanho máximo de uma linha de resposta do dispositivo USB

static struct usb_device *smartlamp_device;        // Referência para o dispositivo USB
static uint usb_in, usb_out;                       // Endereços das portas de entrada e saida da USB
static char *usb_in_buffer, *usb_out_buffer;       // Buffers de entrada e saída da USB
static int usb_max_size;                           // Tamanho máximo de uma mensagem USB

// ATENÇÃO: Substitua os valores abaixo pelos IDs do seu dispositivo!
// Use o comando 'lsusb' para encontrá-los.
#define VENDOR_ID   0x1a86 // Exemplo: VendorID do Silicon Labs CP210x
#define PRODUCT_ID  0x7523 // Exemplo: ProductID do Silicon Labs CP210x
static const struct usb_device_id id_table[] = { { USB_DEVICE(VENDOR_ID, PRODUCT_ID) }, {} };

static int  usb_probe(struct usb_interface *ifce, const struct usb_device_id *id); // Executado quando o dispositivo é conectado na USB
static void usb_disconnect(struct usb_interface *ifce);                            // Executado quando o dispositivo USB é desconectado da USB
static int  usb_write_serial(char *cmd, int param);                                // Executado para enviar um comando para o dispositivo

// Função para configurar os parâmetros seriais do CP2102 via Control-Messages
static int smartlamp_config_serial(struct usb_device *dev)
{
    int ret;
    // Ajustado para corresponder ao baud rate do firmware do ESP32
    u32 baudrate = 115200; 

    printk(KERN_INFO "SmartLamp: Configurando a porta serial...\n");

    // 1. Habilita a interface UART do CP2102
    ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
                          0x00, 0x41, 0x0001, 0, NULL, 0, 1000);
    if (ret)
    {
        printk(KERN_ERR "SmartLamp: Erro ao habilitar a UART (código %d)\n", ret);
        return ret;
    }

    // 2. Define o baud rate
    ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
                          0x1E, 0x41, 0, 0, &baudrate, sizeof(baudrate), 1000);
    if (ret < 0)
    {
        printk(KERN_ERR "SmartLamp: Erro ao configurar o baud rate (código %d)\n", ret);
        return ret;
    }

    printk(KERN_INFO "SmartLamp: Baud rate configurado para %d\n", baudrate);
    return 0;
}

MODULE_DEVICE_TABLE(usb, id_table);
bool ignore = true;

static struct usb_driver smartlamp_driver = {
    .name        = "smartlamp",     // Nome do driver
    .probe       = usb_probe,       // Executado quando o dispositivo é conectado na USB
    .disconnect  = usb_disconnect,  // Executado quando o dispositivo é desconectado na USB
    .id_table    = id_table,        // Tabela com o VendorID e ProductID do dispositivo
};

module_usb_driver(smartlamp_driver);

// Executado quando o dispositivo é conectado na USB
static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    struct usb_endpoint_descriptor *usb_endpoint_in, *usb_endpoint_out;
    int ret;

    printk(KERN_INFO "SmartLamp: Dispositivo conectado ...\n");

    // Detecta portas e aloca buffers de entrada e saída de dados na USB
    smartlamp_device = interface_to_usbdev(interface);
    ignore =  usb_find_common_endpoints(interface->cur_altsetting, &usb_endpoint_in, &usb_endpoint_out, NULL, NULL);
    usb_max_size = usb_endpoint_maxp(usb_endpoint_in);
    usb_in = usb_endpoint_in->bEndpointAddress;
    usb_out = usb_endpoint_out->bEndpointAddress;
    usb_in_buffer = kmalloc(usb_max_size, GFP_KERNEL);
    usb_out_buffer = kmalloc(usb_max_size, GFP_KERNEL);

    // Chama a função para configurar a porta serial antes de usar
    ret = smartlamp_config_serial(smartlamp_device);
    if (ret)
    {
        printk(KERN_ERR "SmartLamp: Falha na configuração da serial\n");
        kfree(usb_in_buffer);
        kfree(usb_out_buffer);
        return ret;
    }

    // Passo 3: Ativar o Envio no usb_probe
    // A linha foi descomentada para chamar a função de escrita.
    ret = usb_write_serial("SET_LED", 100);
    if (ret) {
        printk(KERN_ERR "SmartLamp: Falha ao enviar o comando inicial.\n");
        // A falha aqui não é crítica para a inicialização do driver,
        // então não retornamos erro, apenas logamos.
    }

    return 0;
}

// Executado quando o dispositivo USB é desconectado da USB
static void usb_disconnect(struct usb_interface *interface) {
    printk(KERN_INFO "SmartLamp: Dispositivo desconectado.\n");
    kfree(usb_in_buffer);                   // Desaloca buffers
    kfree(usb_out_buffer);
}

// Envia um comando para o dispositivo USB
static int usb_write_serial(char *cmd, int param) {
    int ret, actual_len;

    printk(KERN_INFO "SmartLamp: Enviando comando: %s %d\n", cmd, param);

    // Passo 2: Implementar usb_write_serial
    // a) Formata o comando no buffer de saída. O formato é "COMANDO PARAMETRO\n".
    sprintf(usb_out_buffer, "%s %d\n", cmd, param);

    // b) Envia o buffer pela USB usando usb_bulk_msg.
    //    - smartlamp_device: O dispositivo USB.
    //    - usb_sndbulkpipe(smartlamp_device, usb_out): Cria o pipe de envio para o endpoint de saída.
    //    - usb_out_buffer: O buffer com os dados a serem enviados.
    //    - strlen(usb_out_buffer): O número de bytes a serem enviados.
    //    - &actual_len: Ponteiro para uma variável que receberá o número de bytes realmente escritos.
    //    - 1000: Timeout em milissegundos.
    ret = usb_bulk_msg(smartlamp_device, usb_sndbulkpipe(smartlamp_device, usb_out),
                       usb_out_buffer, strlen(usb_out_buffer), &actual_len, 1000);

    // c) Verifica o valor de retorno.
    if (ret) {
        printk(KERN_ERR "SmartLamp: Erro ao enviar comando via bulk message (código: %d)\n", ret);
        return ret; // Retorna o código de erro
    }

    // d) Se sucesso, retorna 0.
    printk(KERN_INFO "SmartLamp: Comando enviado com sucesso (%d bytes).\n", actual_len);
    return 0;
}
