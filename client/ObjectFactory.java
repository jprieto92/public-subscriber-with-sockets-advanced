
package client;

import javax.xml.bind.JAXBElement;
import javax.xml.bind.annotation.XmlElementDecl;
import javax.xml.bind.annotation.XmlRegistry;
import javax.xml.namespace.QName;


/**
 * This object contains factory methods for each 
 * Java content interface and Java element interface 
 * generated in the client package. 
 * <p>An ObjectFactory allows you to programatically 
 * construct new instances of the Java representation 
 * for XML content. The Java representation of XML 
 * content can consist of schema derived interfaces 
 * and classes representing the binding of schema 
 * type definitions, element declarations and model 
 * groups.  Factory methods for each of these are 
 * provided in this class.
 * 
 */
@XmlRegistry
public class ObjectFactory {

    private final static QName _TransformText_QNAME = new QName("http://transform/", "transformText");
    private final static QName _TransformTextResponse_QNAME = new QName("http://transform/", "transformTextResponse");

    /**
     * Create a new ObjectFactory that can be used to create new instances of schema derived classes for package: client
     * 
     */
    public ObjectFactory() {
    }

    /**
     * Create an instance of {@link TransformText }
     * 
     */
    public TransformText createTransformText() {
        return new TransformText();
    }

    /**
     * Create an instance of {@link TransformTextResponse }
     * 
     */
    public TransformTextResponse createTransformTextResponse() {
        return new TransformTextResponse();
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}{@link TransformText }{@code >}}
     * 
     */
    @XmlElementDecl(namespace = "http://transform/", name = "transformText")
    public JAXBElement<TransformText> createTransformText(TransformText value) {
        return new JAXBElement<TransformText>(_TransformText_QNAME, TransformText.class, null, value);
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}{@link TransformTextResponse }{@code >}}
     * 
     */
    @XmlElementDecl(namespace = "http://transform/", name = "transformTextResponse")
    public JAXBElement<TransformTextResponse> createTransformTextResponse(TransformTextResponse value) {
        return new JAXBElement<TransformTextResponse>(_TransformTextResponse_QNAME, TransformTextResponse.class, null, value);
    }

}
